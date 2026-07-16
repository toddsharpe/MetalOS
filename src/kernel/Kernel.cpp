#include "kernel/Kernel.h"

//Core crt
#include "core_crt/core_crt.c"

//Platform
#include "kernel/HyperV/Interrupts.cpp"
#include "kernel/HyperV/Timer.cpp"
#include "kernel/HyperV/Tsc.cpp"

//Architecture
#include "x64/CpuId.cpp"

//Kernel
#include "kernel/Kernel_pmem.cpp"
#include "kernel/Kernel_heap.cpp"
#include "kernel/Kernel_vmem.cpp"
#include "kernel/Acpi.cpp"
#include "kernel/Scheduler.cpp"
#include "kernel/ConfigTables.cpp"
#include "kernel/KThread.cpp"
#include "kernel/UProcess.cpp"
#include "kernel/LoadingScreen.cpp"
#include "kernel/Syscalls.cpp"
#include "kernel/UObject.cpp"
#include "kernel/Objects/KPipe.cpp"
#include "kernel/Debugger.cpp"
#include "kernel/AddressSpace.cpp"

//Drivers
#include "kernel/Drivers/VmBusDriver.cpp"
#include "kernel/Drivers/HyperVChannel.cpp"
#include "kernel/Drivers/HyperVRingBuffer.cpp"
#include "kernel/Drivers/HyperVKeyboardDriver.cpp"
#include "kernel/Drivers/HyperVMouseDriver.cpp"
#include "kernel/Drivers/HyperVNic.cpp"
#include "kernel/Drivers/HyperVVideoDriver.cpp"

//Kd64
#include "kernel/Kd64/cpu.cpp"
#include "kernel/Kd64/kdapi.cpp"
#include "kernel/Kd64/kdbreak.cpp"
#include "kernel/Kd64/kddata.cpp"
#include "kernel/Kd64/kdinit.cpp"
#include "kernel/Kd64/kdlock.cpp"
#include "kernel/Kd64/kdprint.cpp"
#include "kernel/Kd64/kdtrap.cpp"
#include "kernel/Kd64/kdx64.cpp"
#include "kernel/Kd64/mmdbg.cpp"
#include "kernel/MetalOSkd.cpp"

//Networking: the TCP/IP stack lives in the usermode netstack process. The kernel keeps
//only the NIC driver (HyperVNic) and a raw ethernet frame device (Kernel_net.cpp).

/*
 * Bootloader parameters.
 */
LoaderParams BootParams;

/*
 * Kernel globals with external linkage. The subsystem files (Kernel_*.cpp) included
 * below reference these through matching `extern` declarations.
 */
DeviceTree m_deviceTree;
KERNEL_PAGE_ALIGN StaticArena<TempArenaSize> m_tempArena;
LinkedList<UProcess*> m_processes;
EFI_RUNTIME_SERVICES m_runtime(*BootParams.Runtime);
Graphics::FrameBuffer m_display((Graphics::Color*)KernelGraphicsDevice, BootParams.Display.VerticalResolution, BootParams.Display.HorizontalResolution);
DirectUart m_uart(DirectUart::ComPort::Com1);
KProcess m_process(KProcessType::Kernel);

namespace
{
	/*
	 * Prototypes.
	 */
	void OnTimer0(void* const arg);
	uint32_t IdleThread(void* unused);

	/*
	 * Kernel loader data (internal to Kernel.cpp).
	 */
	const LoaderParams& m_params(BootParams);
	ConfigTables m_configTables;
	LoadingScreen m_loadingScreen(m_display);

	//System
	InterruptTable<Arch::InterruptVector, Arch::IrqN> m_irqs;
	Debugger m_debugger;

	void Initialize()
	{
		//Initialize architecture
		ArchInitialize();

		//Initialize Display
		m_loadingScreen.Initialize();

		//Initialize components that copy from UEFI space
		m_configTables.Initialize(BootParams.ConfigTables, BootParams.ConfigTablesCount);

		//Initialize physical
		KePhysicalInitialize();

		/*
		 * Initialize Kernel KProcess.
		 * - Create new page tables (requires physical memory to be initialized).
		 */
		m_process.Tables = KernelPageTables::CreateNew();
		m_process.Debug = false;
		Assert(KeVirtualMap(m_process, (void*)KernelBase, m_params.KernelAddress, m_params.KernelImageSize));
		Assert(KeVirtualMap(m_process, (void*)KernelPdb, m_params.PdbAddress, m_params.PdbSize));
		Assert(KeVirtualMap(m_process, (void*)KernelGraphicsDevice, m_params.Display.FrameBufferBase, m_params.Display.FrameBufferSize));
		Assert(KeVirtualMap(m_process, (void*)KernelPageFrameDBStart, m_params.PageFrameAddr, m_params.PageFrameCount * sizeof(PageFrame)));
		Assert(KeVirtualMap(m_process, (void*)KernelPhysicalStart, paddr_t{}, m_params.PageFrameCount << Arch::PageShift));
		//Map UEFI runtime regions into the kernel's page tables
		for (size_t i = 0; i < MaxMemoryRanges; i++)
		{
			const MemoryRange& range = m_params.RuntimeRanges[i];
			if (range.PageCount == 0)
				break;
			Assert(KeVirtualMap(m_process, (void*)(KernelUefiStart + range.Start), range.Start, range.PageCount << Arch::PageShift));
		}

		/*
		 * Adopt new page tbales.
		 * - This removes UEFI identity paging.
		 */
		ArchSetPagingRoot(m_process.Tables.Root);

		//Continue initializing system
		m_irqs.Add(Arch::InterruptVector::Timer0, {&OnTimer0, nullptr});
		Scheduler::Initialize(m_process, &HyperV::Tsc::ReadTsc);
		m_process.Modules.AddModule("moskrnl.exe", (void*)KernelBase);

		//Initialize Platform (HyperV)
		HyperV::Initialize();
		HyperV::Timer::Enable(Second / 128, 0, (uint8_t)Arch::InterruptVector::Timer0);

		//Welcome display
		m_loadingScreen.Printf("MetalOS.Kernel - Base:0x%016x Size: 0x%0X\n", m_params.KernelAddress, m_params.KernelImageSize);
		m_loadingScreen.Printf("  PhysicalAddressSize: 0x%016x\n", m_params.PageFrameCount << Arch::PageShift);

		//Test UEFI runtime access
		EFI_TIME time;
		m_runtime.GetTime(&time, nullptr);
		m_loadingScreen.Printf("  Date: %02d-%02d-%02d %02d:%02d:%02d\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

		//Initialize scheduler and create idle thread. Allows device enumeration to block for interrupts if needed
		KeCreateThread(&IdleThread, nullptr, "Idle");
		Scheduler::Enabled = true;

		//Initialize ACPI/HW
		InitializeAcpi();
		m_deviceTree.Enumerate();
		if (BootParams.RamDriveAddress)
		{
			KDevice* ramDrive = KeAlloc<KDevice>(AllocType::Kernel);
			Assert(ramDrive);
			ramDrive->Hid = KeCopy(RamDriveHid, AllocType::Kernel);
			ramDrive->Type = KDeviceType::Software;
			ramDrive->Context = (void*)BootParams.RamDriveAddress;
			m_deviceTree.AddRootChild(*ramDrive);
		}

		m_debugger.Initialize();
		m_debugger.AddModule(*m_process.Modules.GetModule("moskrnl.exe"));

		//Initialize the kernel->WM input ring. Compositing lives in the usermode
		//WM process (MetalOS-WMSvr.exe), which maps the framebuffer and drains this ring.
		KeInputInitialize();

		//Initialize the kernel net device (RX ring + NIC MAC published for netstack).
		//The TCP/IP stack lives in the usermode netstack process.
		KeInitNetDevice();

		//System displays
		m_process.Display();
		Scheduler::Display();
		m_deviceTree.Display();

		Printf("MetalOS Initialized!\n");
		PrintStack();
	}

	void OnTimer0(void* arg)
	{
		HyperV::Interrupts::EOI();
		if (Scheduler::Enabled)
			Scheduler::Schedule();
	}

	uint32_t IdleThread(void* unused)
	{
		while (true)
			ArchWait();
	}
}

//API split out by subsystem. Included after the kernel globals are defined so that any
//static objects in these files (e.g. _WM::m_backBuffer, built from m_display) initialize
//after the globals they read. Each file declares what it uses via its own `extern`s.
#include "kernel/Kernel_thread.cpp"
#include "kernel/Kernel_file.cpp"
#include "kernel/Kernel_process.cpp"
#include "kernel/Kernel_system.cpp"
#include "kernel/Kernel_diag.cpp"
#include "kernel/Kernel_shm.cpp"
#include "kernel/Kernel_grant.cpp"
#include "kernel/Kernel_endpoint.cpp"
#include "kernel/Kernel_input.cpp"
#include "kernel/Kernel_net.cpp"

void KeRegisterInterrupt(const Arch::InterruptVector interrupt, const ActionContext& context)
{
	Assert(!m_irqs.Contains(interrupt));
	m_irqs.Add(interrupt, context);
}

void* GetAcpiTable()
{
	return m_configTables.GetAcpiTable();
}

//This works for static code/data that is part of kernel image
paddr_t ResolveImageVA(void* const address)
{
	const uint64_t rva = (uintptr_t)address - KernelBase;
	Assert(rva < BootParams.KernelImageSize);
	return m_params.KernelAddress + rva;
}

KDevice* KeGetDevice(const char* const path)
{
	return m_deviceTree.GetDevice(CString(path));
}

//Physical framebuffer descriptor from the loader, for mapping into a process.
KFramebufferInfo KeGetFramebuffer()
{
	return {
		BootParams.Display.FrameBufferBase,
		BootParams.Display.FrameBufferSize,
		BootParams.Display.HorizontalResolution,
		BootParams.Display.VerticalResolution,
		BootParams.Display.PixelsPerScanLine,
	};
}

/*
 * Globals with C linkage.
 */
extern "C"
{
	void KeInterrupt(const Arch::InterruptVector vector, Arch::InterruptFrame& frame)
	{
		if (vector == Arch::InterruptVector::DoubleFault)
		{
			Printf("DOUBLE FAULT RIP=0x%016x RSP=0x%016x\n", frame.RIP, frame.RSP);
			KePauseSystem();
			__halt();
		}

		if (m_irqs.Contains(vector))
		{
			const ActionContext ctx = m_irqs[vector];
			ctx.Invoke();
			return;
		}

		//Check if interrupt is in user code or kernel code
		KThread& current = Scheduler::GetCurrentThread();
		if (current.UserThread && current.UserThread->Process.Space.IsValidPointer((void*)frame.RIP))
		{
			UProcess& proc = current.UserThread->Process;
			UObject* obj = proc.GetObject((handle_t)Handles::StdOut);
			if (obj && obj->Type == UObjectType::Pipe)
			{
				KPipe& pipe = obj->Pipe->KPipe;

				//Write exception
				switch (vector)
				{
					case Arch::InterruptVector::DivideError:
						pipe.Print("Exception: Divide by zero\n");
						break;

					case Arch::InterruptVector::Breakpoint:
						pipe.Print("Exception: Breakpoint\n");
						break;

					case Arch::InterruptVector::PageFault:
						const uint64_t addr = __readcr2();
						pipe.Print("Exception: Invalid virtual address: 0x%016x. %s\n", addr, addr == 0 ? "Null pointer" : "");
						break;
				}

				//Convert context
				//TODO(tsharpe): Remove this
				CONTEXT ctx = {};
				ctx.Rip = frame.RIP;
				ctx.Rsp = frame.RSP;
				ctx.Rbp = frame.RBP;

				//Reset temp arena
				m_tempArena.Reset();

				//Print stack
				StaticMap<const KModule*, PdbFile*, 32> map;
				StackWalk sw(&ctx);
				while (sw.HasNext())
				{
					//Get module
					const KModule* module = proc.Modules.GetModule(ctx.Rip);
					if (module)
					{
						PdbFile* pdb = nullptr;
						if (!map.Get(module, pdb))
						{
							//Get PDB name
							const char* fullPath = WinPE::GetPdbName(module->ImageBase);
							Assert(fullPath);
							const char* pdbName = GetFileName(fullPath);
							Assert(pdbName);

							//Load PDB into memory
							void* const loaded = KeLoadFile(pdbName);

							//Load PdbFile
							pdb = m_tempArena.Allocate<PdbFile>();
							pdb->Open(loaded, module->ImageBase, m_tempArena);
							map.Add(module, pdb);
						}

						//Calculate RVA
						const uint32_t rva = (uint32_t)(ctx.Rip - (uintptr_t)module->ImageBase);

						//Lookup in PDB
						PdbLookup lookup = {};
						bool success = pdb->Resolve(rva, lookup);
						if (success)
						{
							pipe.Print("    %s (%d)\n", lookup.Function.c_str(), lookup.Line);
						}
						else
						{
							pipe.Print("    <resolve failed>\n");
						}
						pipe.Print("        IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", ctx.Rip, module->ImageBase, rva);
					}
					else
					{
						//No module info for the IP, print it and exit
						pipe.Print("    IP: 0x%016x\n", ctx.Rip);
						break;
					}
					sw.Next((uintptr_t)module->ImageBase);
				}
			}

			//Kill process
			KeTerminateProcess(proc, -1);
			Unreachable();
		}
		else
		{
			//Show context
			Printf("Vector: %d\n", vector);
			Printf("ISR: 0x%x, Code: %x\n", vector, frame.ErrorCode);
			Printf("    RIP: 0x%016x\n", frame.RIP);
			Printf("    RBP: 0x%016x\n", frame.RBP);
			Printf("    RSP: 0x%016x\n", frame.RSP);
			Printf("    RAX: 0x%016x\n", frame.RAX);
			Printf("    RBX: 0x%016x\n", frame.RBX);
			Printf("    RCX: 0x%016x\n", frame.RCX);
			Printf("    RDX: 0x%016x\n", frame.RDX);
			Printf("    CS: 0x%x, SS: 0x%x\n", frame.CS, frame.SS);

			switch (vector)
			{
			case Arch::InterruptVector::PageFault:
				Printf("    CR2: 0x%16x\n", __readcr2());
				if (__readcr2() == 0)
					Printf("        Null pointer\n");
			}

			//Attempt to pass to debugger
			if (m_debugger.Enabled())
			{
				m_debugger.DebuggerEvent(vector, frame);
				return;
			}
			
			//Build context
			Arch::Context context = {};
			context.Rip = frame.RIP;
			context.Rsp = frame.RSP;
			context.Rbp = frame.RBP;

			//Interrupt originated in kernel
			PrintStack(&context, m_process);
			Fatal("Unhandled exception");
		}
	}

	int KeMain()
	{
		Initialize();

		KeCreateProcess("init.exe from_kernel");
		KeExitThread();

		return 0;
	}
}