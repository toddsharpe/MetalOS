#include "kernel/Kernel.h"
#include "kernel/System.h"
#include "kernel/Pmem.h"
#include "kernel/Vmem.h"
#include "kernel/Heap.h"
#include "kernel/Thread.h"
#include "kernel/Process.h"
#include "kernel/Diag.h"
#include "kernel/Net.h"
#include "kernel/Input.h"
#include "kernel/Arch.h"
#include "kernel/Plat.h"
#include "kernel/Devices/Acpi.h"
#include "kernel/Core/KProcess.h"
#include "kernel/Core/UProcess.h"
#include "kernel/Core/Scheduler.h"
#include "kernel/Devices/DeviceTree.h"
#include "kernel/Debug/Debugger.h"
#include "kernel/LoadingScreen.h"
#include "kernel/Core/InterruptTable.h"
#include "kernel/File.h"
#include "MetalOS.Loader.h"
#include "MetalOS.Space.h"
#include "Graphics/Types.h"
#include "WinPE.h"
#include "Lib/String.h"
#include "Assert.h"

//Networking: the TCP/IP stack lives in the usermode netstack process. The kernel keeps
//only the NIC driver (HyperVNic) and a raw ethernet frame device (kernel/Net.cpp).

/*
 * Bootloader parameters.
 */
LoaderParams BootParams;

//Kernel globals shared with the other kernel files through Kernel.h
DeviceTree m_deviceTree;
static Graphics::FrameBuffer m_display((Graphics::Color*)KernelGraphicsDevice, BootParams.Display.VerticalResolution, BootParams.Display.HorizontalResolution);
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
		Scheduler::Initialize(m_process, &PlatReadTime);
		const KModule* const kernelModule = KeAddModule(Str("moskrnl.exe"), (void*)KernelBase, WinPE::GetSizeOfImage((void*)KernelBase));
		Assert(kernelModule);

		//Initialize platform
		PlatInitialize();
		PlatStartTimer(Second / 128, (uint8_t)Arch::InterruptVector::Timer0);

		//Welcome display
		m_loadingScreen.Printf("MetalOS.Kernel - Base:0x%016x Size: 0x%0X\n", m_params.KernelAddress, m_params.KernelImageSize);
		m_loadingScreen.Printf("  PhysicalAddressSize: 0x%016x\n", m_params.PageFrameCount << Arch::PageShift);

		//Test UEFI runtime access
		KSystemTime time;
		KeGetSystemTime(time);
		m_loadingScreen.Printf("  Date: %02d-%02d-%02d %02d:%02d:%02d\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

		//Initialize scheduler and create idle thread. Allows device enumeration to block for interrupts if needed
		Scheduler::SetIdleThread(*KeCreateThread(&IdleThread, nullptr, "Idle"));
		KeCreateThread(&KeReaperThread, nullptr, "Reaper");
		Scheduler::Enable();

		//Initialize ACPI/HW
		InitializeAcpi();
		m_deviceTree.Enumerate();
		KeMountStorage();

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
		UNUSED(arg);
		PlatEndOfInterrupt();
		if (Scheduler::IsEnabled())
			Scheduler::Schedule();
	}

	uint32_t IdleThread(void* unused)
	{
		UNUSED(unused);
		while (true)
			ArchWait();
	}
}


void KeRegisterInterrupt(const Arch::InterruptVector interrupt, const ActionContext& context)
{
	Assert(!m_irqs.Contains(interrupt));
	m_irqs.Add(interrupt, context);
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
			Printf("DOUBLE FAULT\n");
			ArchPrintFrame(frame);
			KePauseSystem();
			ArchWait();
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
					const uint64_t addr = ArchFaultAddress();
					TracePrint(pipe, "Exception: Invalid virtual address: 0x%016x. %s\n", addr, addr == 0 ? "Null pointer" : "");
				}
				break;

				default:
					TracePrint(pipe, "Exception: Vector 0x%x, Code: 0x%x\n", (uint32_t)vector, ArchFrameErrorCode(frame));
					break;
			}

			//Print the faulting thread's stack
			const Arch::Context context = ArchFrameContext(frame);
			PrintStack(&context, proc, pipe);

			//Kill process
			KeTerminateProcess(proc, UINT32_MAX);
			Unreachable();
		}
		else
		{
			//Show context
			Printf("Vector: %d\n", vector);
			Printf("ISR: 0x%x, Code: %x\n", vector, ArchFrameErrorCode(frame));
			ArchPrintFrame(frame);

			switch (vector)
			{
			case Arch::InterruptVector::PageFault:
				Printf("    Fault address: 0x%16x\n", ArchFaultAddress());
				if (ArchFaultAddress() == 0)
					Printf("        Null pointer\n");
			}

			//Attempt to pass to debugger, falling through to the stack trace when no host answers
			if (m_debugger.Enabled())
			{
				m_debugger.DebuggerEvent(vector, frame);
				if (m_debugger.HostPresent())
					return;
			}
			
			//Interrupt originated in kernel
			const Arch::Context context = ArchFrameContext(frame);
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