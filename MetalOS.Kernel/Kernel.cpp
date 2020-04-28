#include "Kernel.h"

typedef EFI_GUID GUID;
#define PACKED
#include <PlatformAcpi.h>
#include <intrin.h>
#include <cstdarg>
#include "x64.h"
#include "x64_support.h"
#include "System.h"
#include "Main.h"
#include <functional>
#include "AcpiDevice.h"
#include "UartDriver.h"
#include "RuntimeSupport.h"
#include "LoadingScreen.h"
#include "RtcDriver.h"
#include "ProcessorDriver.h"
#include "HyperVTimer.h"
#include "HyperV.h"

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Kernel::Kernel() :
	m_physicalAddress(0),
	m_imageSize(0),
	m_runtime(),
	m_pPagePool(nullptr),
	m_pMemoryMap(nullptr),
	m_pConfigTables(nullptr),
	m_pageTables(nullptr),
	m_pDisplay(nullptr),
	m_textScreen(),
	m_lastProcessId(0),
	m_processes(nullptr),
	m_objectId(0),
	m_deviceTree()
{

}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define Trace() Print(__FILE__ "-" STR(__LINE__));
void Kernel::Initialize(const PLOADER_PARAMS params)
{
	//Save Loader Params
	m_physicalAddress = params->KernelAddress;
	m_imageSize = params->KernelImageSize;
	m_runtime = *params->Runtime;//UEFI has rewritten these pointers, now we copy them locally

	//Initialize Heap

	//Initialize Display
	m_pDisplay = new Display(params->Display, KernelGraphicsDeviceAddress);
	m_pDisplay->ColorScreen(Black);
	m_textScreen = new TextScreen(*m_pDisplay);
	m_printer = m_textScreen;

	Print("Flags: 0x%016x\n", x64_rflags());

	//Initialize memory map
	m_pMemoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, params->MemoryMap);
	m_pMemoryMap->ReclaimBootPages();
	m_pMemoryMap->MergeConventionalPages();

	//Config Tables
	m_pConfigTables = new ConfigTables(params->ConfigTables, params->ConfigTableSizes);

	//Initialize page table pool
	m_pPagePool = new PageTablesPool(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);

	//Construct kernel page table
	UINT64 ptRoot;
	Assert(m_pPagePool->AllocatePage(&ptRoot));
	m_pageTables = new PageTables(ptRoot);
	m_pageTables->SetPool(m_pPagePool);
	m_pageTables->MapKernelPages(KernelBaseAddress, params->KernelAddress, EFI_SIZE_TO_PAGES(params->KernelImageSize));
	m_pageTables->MapKernelPages(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	m_pageTables->MapKernelPages(KernelGraphicsDeviceAddress, params->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(params->Display.FrameBufferSize));
	m_pageTables->MapKernelPages(KernelPfnDbStart, params->PfnDbAddress, EFI_SIZE_TO_PAGES(params->PfnDbSize));

	//Map in runtime addresses
	{
		EFI_MEMORY_DESCRIPTOR* current;
		for (current = params->MemoryMap;
			current < NextMemoryDescriptor(params->MemoryMap, params->MemoryMapSize);
			current = NextMemoryDescriptor(current, params->MemoryMapDescriptorSize))
		{
			if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
				continue;

			Assert(m_pageTables->MapKernelPages(current->VirtualStart, current->PhysicalStart, current->NumberOfPages));
		}
	}

	//Use new Kernel PT
	//Identity mappings are not possible now, lost access to params
	//Alternatively - remove physical identity mappings from bootloader PT since entry in pt root can be cleared and subsequent pages are going to be eaten (since they are in boot memory)
	//This requires copying at least the root
	m_pfnDbSize = params->PfnDbSize;
	m_pfnDbAddress = params->PfnDbAddress;
	__writecr3(ptRoot);

	//Test UEFI runtime access
	EFI_TIME time = { 0 };
	m_runtime.GetTime(&time, nullptr);
	Print("Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Test interrupts
	//__debugbreak();

	Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x\n", m_physicalAddress, m_imageSize);
	//Print("ConfigTableSizes: %d", loader->ConfigTableSizes);
	Print("  PhysicalAddressSize: 0x%16x\n", m_pMemoryMap->GetPhysicalAddressSize());
	//Print("  PageTablesPool.AllocatedPageCount: 0x%8x", m_pPagePool->AllocatedPageCount());

	//Initialize PFN database
	m_pfnDb = new PhysicalMemoryManager(*m_pMemoryMap);

	//Complete initialization
	m_processes = new std::list<KERNEL_PROCESS>();
	m_interruptHandlers = new std::map<InterruptVector, IrqHandler>();
	m_threads = new std::map<uint32_t, KernelThread*>();
	m_virtualMemory = new VirtualMemoryManager(*m_pfnDb, *m_pPagePool);
	m_addressSpace = new VirtualAddressSpace(KernelRuntimeStart, KernelRuntimeEnd, true);
	m_readyQueue = new std::queue<uint32_t>();
	m_scheduler = new Scheduler();

	//Initialize Hyperv
	m_hyperV = new HyperV();
	Assert(m_hyperV->IsPresent());
	Assert(m_hyperV->DirectSyntheticTimers());
	Assert(m_hyperV->AccessPartitionReferenceCounter());
	m_hyperV->Initialize();
	Print("TscFreq: 0x%016x\n", m_hyperV->TscFreq());

	//Initialize our boot thread
	KernelThread* current = new KernelThread();
	memset(current, 0, sizeof(KernelThread));
	current->Id = ++m_lastId;
	current->State = ThreadState::Running;
	current->Context = new uint8_t[x64_CONTEXT_SIZE];
	current->TEB = new ThreadEnvironmentBlock();
	current->TEB->SelfPointer = current->TEB;
	current->TEB->ThreadId = current->Id;
	m_threads->insert({ current->Id, current });
	x64::SetKernelTEB(current->TEB);
	x64_swapgs();

	//Initialized IO
	this->InitializeAcpi();

	//Devices
	m_deviceTree.Populate();

	//Swap output to uart
	AcpiDevice* com1;
	Assert(m_deviceTree.GetDeviceByName("COM1", &com1));
	this->m_printer = ((UartDriver*)com1->GetDriver());
	Print("COM1 initialized\n");
	
	//Show loading screen
	LoadingScreen loadingScreen(*m_pDisplay);
	loadingScreen.Initialize();
	//__halt();

	//Output full current state
	m_pMemoryMap->DumpMemoryMap();
	m_pConfigTables->Dump();
	m_deviceTree.Display();

	//Output from drivers
	//AcpiDevice* rtc;
	//Assert(m_deviceTree.GetDeviceByHid("PNP0B00", &rtc));
	//RtcDriver* rtcDriver = ((RtcDriver*)rtc->GetDriver());
	//rtcDriver->Display();
	//rtcDriver->Enable();
	//rtcDriver->Display();

	//Apic
	//AcpiDevice* proc;
	//Assert(m_deviceTree.GetDeviceByHid("ACPI0007", &proc));
	//ProcessorDriver* procDriver = ((ProcessorDriver*)proc->GetDriver());
	//procDriver->Display();

	m_interruptHandlers->insert({ InterruptVector::Timer0, &Kernel::OnTimer0 });
	m_timer = new HyperVTimer(0);
	m_timer->SetPeriodic(SECOND / 128, InterruptVector::Timer0);

	//Done
	Print("Kernel Initialized\n");
}

void Kernel::HandleInterrupt(InterruptVector vector, PINTERRUPT_FRAME pFrame)
{
	if (vector == InterruptVector::DoubleFault)
		__halt();

	const auto& it = m_interruptHandlers->find(vector);
	if (it != m_interruptHandlers->end())
	{
		IrqHandler h = it->second;
		(this->*h)(pFrame);
		return;
	}
	
	m_textScreen->Printf("ISR: 0x%x, Code: %d, RBP: 0x%16x, RIP: 0x%16x, RSP: 0x%16x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	m_textScreen->Printf("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	switch (vector)
	{
	//Let debug continue (we use this to check ISRs on bootup)
	case 3:
		return;
	case 14:
		m_textScreen->Printf("CR2: 0x%16x\n", __readcr2());
		if (__readcr2() == 0)
			m_textScreen->Printf("Null pointer dereference\n", __readcr2());
	}

	//TODO: stack walk
}

void Kernel::Bugcheck(const char* file, const char* line, const char* assert)
{
	//display.ColorScreen(Red);
	//TODO: color background function (pass lambda bool to colorscreen)

	//loading->ResetX();
	//loading->ResetY();
	m_textScreen->Printf("%s\n%s\n%s", file, line, assert);

	__halt();
}

void Kernel::Printf(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	this->Printf(format, args);
	va_end(args);
}

void Kernel::Printf(const char* format, va_list args)
{
	m_printer->Printf(format, args);
}

void Kernel::InitializeAcpi()
{
	ACPI_STATUS Status;
	Status = AcpiInitializeSubsystem();
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeSubsystem: %d\n", Status);
		__halt();
	}

	Status = AcpiInitializeTables(nullptr, 16, FALSE);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeTables: %d\n", Status);
		__halt();
	}

	//TODO: notify handlers

	/* Install the default address space handlers. */
	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_MEMORY, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not initialise SystemMemory handler, %s!", AcpiFormatException(Status));
		__halt();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_IO, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not initialise SystemIO handler, %s!", AcpiFormatException(Status));
		__halt();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_PCI_CONFIG, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not initialise PciConfig handler, %s!", AcpiFormatException(Status));
		__halt();
	}

	Status = AcpiLoadTables();
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiLoadTables: %d\n", Status);
		__halt();
	}

	//Local handlers should be installed here

	Status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiEnableSubsystem: %d\n", Status);
		__halt();
	}

	Status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeObjects: %d\n", Status);
		__halt();
	}

	Print("ACPI Finished\n");
}

void Kernel::OnTimer0(void* arg)
{
	cpu_flags_t flags = x64_disable_interrupts();
	HyperV::EOI();

	m_scheduler->Schedule();
	x64_restore_flags(flags);
}

//Kernel threads are setup as if an interrupt occured (so our interrupt handler can switch between them)
void Kernel::CreateThread(ThreadStart start, void* arg)
{
	//Print("Kernel::CreateThread\n");

	KernelThread* thread = new KernelThread();
	memset(thread, 0, sizeof(KernelThread));
	thread->Id = ++m_lastId;
	thread->State == ThreadState::Ready;
	thread->Start = start;
	thread->Arg = arg;
	thread->TEB = new ThreadEnvironmentBlock();
	thread->TEB->SelfPointer = thread->TEB;
	thread->TEB->ThreadId = thread->Id;
	m_threads->insert({ thread->Id, thread });

	//Create thread stack, point to bottom (stack grows shallow)
	uintptr_t stack = (uintptr_t)m_virtualMemory->Allocate(0, KERNEL_THREAD_STACK_SIZE, MemoryProtection(true, true, false), *m_addressSpace);
	stack += (KERNEL_THREAD_STACK_SIZE << PAGE_SHIFT);//Since pop reads first, subtract stack width to be on valid address

	//Subtract paramater area
	stack -= 32;

	//Allocate space for context
	thread->Context = new uint8_t[x64_CONTEXT_SIZE];
	x64_init_context(thread->Context, (void*)stack, &Kernel::ThreadInitThunk);

	//Add to scheduler
	m_scheduler->Add(*thread);
}

void Kernel::ThreadInitThunk()
{
	//Print("Kernel::ThreadInitThunk\n");
	KernelThread* current = kernel.GetCurrentThread();
	//Print("Start: 0x%016x, Arg: 0x%016x\n", current->Start, current->Arg);
	
	//Run thread
	current->Start(current->Arg);

	//Exit thread TODO
	__halt();
}

void Kernel::Sleep(nano_t value)
{
	m_scheduler->Sleep(value);
}

ThreadEnvironmentBlock* Kernel::GetTEB()
{
	return (ThreadEnvironmentBlock*)__readgsqword(offsetof(ThreadEnvironmentBlock, SelfPointer));
}

KernelThread* Kernel::GetKernelThread(uint32_t threadId)
{
	const auto& it = m_threads->find(threadId);
	Assert(it != m_threads->end());
	return it->second;
}

//Uses GS register and does lookup, ensuring both exist
KernelThread* Kernel::GetCurrentThread()
{
	ThreadEnvironmentBlock* teb = kernel.GetTEB();
	Assert(teb);
	KernelThread* current = kernel.GetKernelThread(teb->ThreadId);
	Assert(current);
	return current;
}

void Kernel::GetSystemTime(SystemTime* time)
{
	EFI_TIME efiTime = { 0 };
	Assert(!EFI_ERROR(m_runtime.GetTime(&efiTime, nullptr)));

	time->Year = efiTime.Year;
	time->Month = efiTime.Month;
	//TODO: day of the week
	time->Day = efiTime.Day;
	time->Hour = efiTime.Hour;
	time->Minute = efiTime.Minute;
	time->Second = efiTime.Second;
	time->Milliseconds = efiTime.Nanosecond / 1000;
}
