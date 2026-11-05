#include "kernel/Kernel.h"

//Core crt
#include "core_crt/core_crt.c"
#include "core_crt/wchar.c"

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

		//Initialize physical
		KePhysicalInitialize();

		/*
		 * Initialize Kernel KProcess.
		 * - Create new page tables (requires physical memory to be initialized).
		 */
		m_process.Tables = KernelPageTables::CreateNew();
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
		const KModule* const kernelModule = KeAddModule("moskrnl.exe", (void*)KernelBase, WinPE::GetSizeOfImage((void*)KernelBase));
		Assert(kernelModule);

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
		Scheduler::SetIdleThread(*KeCreateThread(&IdleThread, nullptr, "Idle"));
		KeCreateThread(&KeReaperThread, nullptr, "Reaper");
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
		m_debugger.AddModule(*kernelModule);

		//Initialize the kernel->WM input ring. Compositing lives in the usermode
		//WM process (MetalOS-WMSvr.exe), which maps the framebuffer and drains this ring.
		KeInputInitialize();

		//Initialize the kernel net device: publishes the RX ring for the usermode netstack, which runs TCP/IP
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

//API split out by subsystem, included after the kernel globals so any statics in these files initialize after the globals they read
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

//This works for static code/data that is part of kernel image
paddr_t ResolveImageVA(void* const address)
{
	const uint64_t rva = (uintptr_t)address - KernelBase;
	Assert(rva < BootParams.KernelImageSize);
	return m_params.KernelAddress + rva;
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

		//User or kernel fault by the interrupted CS's privilege level (not RIP: a user jump to null is a user fault)
		KThread& current = Scheduler::GetCurrentThread();
		const bool fromUser = (frame.CS & 0x3) == 0x3;
		if (fromUser && current.UserThread)
		{
			UProcess& proc = current.UserThread->Process;

			//The report always goes to the kernel console, and to stderr as well when that is a pipe (a Debug stderr is the console)
			UObject* const obj = proc.GetObject((handle_t)Handles::StdErr);
			KPipe* const pipe = (obj && obj->Type == UObjectType::Pipe) ? obj->Pipe : nullptr;

			//Write exception
			switch (vector)
			{
				case Arch::InterruptVector::DivideError:
					TracePrint(pipe, "Exception: Divide by zero\n");
					break;

				case Arch::InterruptVector::Breakpoint:
					TracePrint(pipe, "Exception: Breakpoint\n");
					break;

				case Arch::InterruptVector::PageFault:
				{
					const uint64_t addr = __readcr2();
					TracePrint(pipe, "Exception: Invalid virtual address: 0x%016x. %s\n", addr, addr == 0 ? "Null pointer" : "");
				}
				break;

				default:
					TracePrint(pipe, "Exception: Vector 0x%x, Code: 0x%x\n", (uint32_t)vector, frame.ErrorCode);
					break;
			}

			//Print the faulting thread's stack
			Arch::Context context = {};
			context.Rip = frame.RIP;
			context.Rsp = frame.RSP;
			context.Rbp = frame.RBP;
			PrintStack(&context, proc, pipe);

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

			//Attempt to pass to debugger, falling through to the stack trace when no host answers
			if (m_debugger.Enabled())
			{
				m_debugger.DebuggerEvent(vector, frame);
				if (m_debugger.HostPresent())
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

		//Every other process descends from init, so without it the system can only idle
		if (!KeCreateProcess("init.exe from_kernel"))
			Fatal("init.exe failed to start");
		KeExitThread();

		return 0;
	}
}