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
#include "KernelHeap.h"
#include "BootHeap.h"
#include "StackWalk.h"
#include "KSemaphore.h"
#include "RamDriveDriver.h"
#include "SoftwareDevice.h"
#include "Loader.h"
#include <string>

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Kernel::Kernel() :
	m_physicalAddress(),
	m_imageSize(),
	m_runtime(),
	m_pdbAddress(),
	m_pdbSize(),

	m_display(),
	m_textScreen(),
	m_printer(),

	m_memoryMap(),
	m_configTables(),
	m_pagePool(),
	m_pageTables(),

	m_heapInitialized(),
	m_pfnDbAddress(),
	m_pfnDb(),
	m_virtualMemory(),
	m_addressSpace(),
	m_heap(),

	m_interruptHandlers(),

	m_processes(),
	m_lastId(),
	m_threads(),
	m_scheduler(),

	m_hyperV(),

	m_deviceTree(),

	m_timer(),

	m_pdb()
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
	m_pfnDbAddress = params->PfnDbAddress;
	const size_t pfnDbSize = params->PfnDbSize;
	m_pdbAddress = params->PdbAddress;
	m_pdbSize = params->PdbSize;
	const paddr_t ramDriveAddress = params->RamDriveAddress;

	//Initialize Display
	m_display = new Display(params->Display, KernelGraphicsDeviceAddress);
	m_display->ColorScreen(Black);
	m_textScreen = new TextScreen(*m_display);
	m_printer = m_textScreen;

	//Initialize and process memory map
	m_memoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, params->MemoryMap);
	m_memoryMap->ReclaimBootPages();
	m_memoryMap->MergeConventionalPages();

	//UEFI configuration tables
	m_configTables = new ConfigTables(params->ConfigTables, params->ConfigTableSizes);

	//Initialize page table pool
	m_pagePool = new PageTablesPool(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);

	//Construct kernel page table
	UINT64 ptRoot;
	Assert(m_pagePool->AllocatePage(&ptRoot));
	m_pageTables = new PageTables(ptRoot);
	m_pageTables->SetPool(m_pagePool);
	m_pageTables->MapKernelPages(KernelBaseAddress, params->KernelAddress, EFI_SIZE_TO_PAGES(params->KernelImageSize));
	m_pageTables->MapKernelPages(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	m_pageTables->MapKernelPages(KernelGraphicsDeviceAddress, params->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(params->Display.FrameBufferSize));
	m_pageTables->MapKernelPages(KernelPfnDbStart, params->PfnDbAddress, EFI_SIZE_TO_PAGES(params->PfnDbSize));
	m_pageTables->MapKernelPages(KernelPdbStart, params->PdbAddress, EFI_SIZE_TO_PAGES(params->PdbSize));

	//Map in runtime sections
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
	__writecr3(ptRoot);

	Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x\n", m_physicalAddress, m_imageSize);
	Print("  PhysicalAddressSize: 0x%16x\n", m_memoryMap->GetPhysicalAddressSize());

	//Test UEFI runtime access
	EFI_TIME time = { 0 };
	m_runtime.GetTime(&time, nullptr);
	Print("  Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Initialize virtual memory
	m_pfnDb = new PhysicalMemoryManager(*m_memoryMap);
	Assert(m_pfnDb->GetSize() == pfnDbSize);
	m_virtualMemory = new VirtualMemoryManager(*m_pfnDb, *m_pagePool);
	m_addressSpace = new VirtualAddressSpace(KernelRuntimeStart, KernelRuntimeEnd, true);

	//Initialize Heap
	m_heap = new KernelHeap(*m_virtualMemory, *m_addressSpace);
	m_heapInitialized = true;

	//PDB
	m_pdb = new Pdb(KernelPdbStart);

	//Interrupts
	m_interruptHandlers = new std::map<InterruptVector, InterruptContext>();
	m_interruptHandlers->insert({ InterruptVector::Timer0, { &Kernel::OnTimer0, this} });

	//Test interrupts
	__debugbreak();

	m_processes = new std::list<KernelProcess*>();
	m_threads = new std::map<uint32_t, KernelThread*>();
	m_scheduler = new Scheduler();

	KernelProcess* process = new KernelProcess();
	memset(process, 0, sizeof(KernelProcess));
	m_processes->push_back(process);
	process->Id = ++m_lastId;
	process->CR3 = ptRoot;
	process->VirtualAddress = m_addressSpace;
	strcpy(process->Name, "moskrnl.exe");

	//Initialize boot thread
	KernelThread* current = new KernelThread();
	memset(current, 0, sizeof(KernelThread));
	current->Process = process;
	current->Id = ++m_lastId;
	current->State = ThreadState::Running;
	current->Context = new uint8_t[x64_CONTEXT_SIZE];
	current->TEB = new ThreadEnvironmentBlock();
	current->TEB->SelfPointer = current->TEB;
	current->TEB->ThreadId = current->Id;
	m_threads->insert({ current->Id, current });
	x64::SetKernelTEB(current->TEB);
	x64_swapgs();

	//Create idle thread
	kernel.CreateThread(&Kernel::IdleThread, this);

	//Initialize Platform
	m_hyperV = new HyperV();
	Assert(m_hyperV->IsPresent());
	Assert(m_hyperV->DirectSyntheticTimers());
	Assert(m_hyperV->AccessPartitionReferenceCounter());
	Assert(!m_hyperV->DeprecateAutoEOI());
	m_hyperV->Initialize();

	//Initialized IO
	this->InitializeAcpi();

	//Devices
	m_deviceTree.Populate();

	if (ramDriveAddress != NULL)
		m_deviceTree.AddRootDevice(*new SoftwareDevice(RamDriveHid, (void*)ramDriveAddress));

	//Swap output to uart
	Device* com1;
	Assert(m_deviceTree.GetDeviceByName("COM1", &com1));
	this->m_printer = ((UartDriver*)com1->GetDriver());

	//Output full current state
	m_memoryMap->DumpMemoryMap();
	//m_configTables->Dump();
	//m_deviceTree.Display();
	//m_heap->PrintHeap();
	m_pageTables->DisplayCr3();

	//Scheduler (needed to load VMBus driver)
	m_scheduler->Enabled = true;
	m_timer = new HyperVTimer(0);
	m_timer->SetPeriodic(SECOND / 64, (uint8_t)InterruptVector::Timer0);
	//m_timer->SetPeriodic(SECOND, InterruptVector::Timer0);

	//Attach drivers and enumerate tree
	m_deviceTree.EnumerateChildren();
	m_deviceTree.Display();

	//Show loading screen (works but we don't need yet)
	//LoadingScreen loadingScreen(*m_display);
	//loadingScreen.Initialize();

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
		InterruptContext ctx = it->second;
		ctx.Handler(ctx.Context);
		return;
	}
	
	m_textScreen->Printf("ISR: 0x%x, Code: %d, RBP: 0x%16x, RIP: 0x%16x, RSP: 0x%16x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	m_textScreen->Printf("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	m_textScreen->Printf("   CS: 0x%16x,  SS: 0x%16x,  GS: 0x%16x,  FS: 0x%16x\n", pFrame->CS, pFrame->SS, x64_ReadGS(), x64_ReadFS());
	switch (vector)
	{
	//Let debug continue (we use this to check ISRs on bootup)
	case InterruptVector::Breakpoint:
		m_textScreen->Printf("  Debug Breakpoint Exception\n");
		return;
	case InterruptVector::PageFault:
		m_textScreen->Printf("  CR2: 0x%16x\n", __readcr2());
		if (__readcr2() == 0)
			m_textScreen->Printf("  Null pointer dereference\n", __readcr2());
	}

	if (m_scheduler != nullptr)
		m_scheduler->Enabled = false;
	if (m_timer != nullptr)
		m_timer->Disable();

	//m_heap->PrintHeap();

	CONTEXT context = { 0 };
	context.Rip = pFrame->RIP;
	context.Rsp = pFrame->RSP;
	context.Rbp = pFrame->RBP;

	StackWalk sw(&context, KernelBaseAddress);
	uint32_t rva = (uint32_t)context.Rip - KernelBaseAddress;

	while (sw.HasNext())
	{
		Print("IP: 0x%016x ", context.Rip);
		if (m_pdb != nullptr)
			m_pdb->PrintStack((uint32_t)context.Rip - KernelBaseAddress);
		else
			Print("\n");

		sw.Next();
	}
	__halt();
}

bool inBugcheck = false;
void Kernel::Bugcheck(const char* file, const char* line, const char* assert)
{
	if (inBugcheck)
	{
		m_textScreen->Printf("%s\n%s\n%s", file, line, assert);

		while(true)
			__halt();
	}
	
	inBugcheck = true;
	
	if (m_scheduler != nullptr)
		m_scheduler->Enabled = false;

	if (m_timer != nullptr)
		m_timer->Disable();

	m_textScreen->Printf("%s\n%s\n%s", file, line, assert);

	uint8_t buffer[sizeof(x64_context)];
	Assert(sizeof(x64_context) == x64_CONTEXT_SIZE);
	x64_save_context(buffer);

	x64_context* x64context = (x64_context*)buffer;

	CONTEXT context = { 0 };
	context.Rip = x64context->Rip;
	context.Rsp = x64context->Rsp;
	context.Rbp = x64context->Rbp;

	StackWalk sw(&context, KernelBaseAddress);
	uint32_t rva = (uint32_t)context.Rip - KernelBaseAddress;
	
	while (sw.HasNext())
	{
		Print("IP: 0x%016x ", context.Rip);
		if (m_pdb != nullptr)
			m_pdb->PrintStack((uint32_t)context.Rip - KernelBaseAddress);
		else
			Print("\n");

		sw.Next();
	}

	//m_display->ColorScreen(Red);

	//m_textScreen->ResetX();
	//m_textScreen->ResetY();

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
	if (m_printer != nullptr)
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

void Kernel::OnTimer0()
{
	cpu_flags_t flags = x64_disable_interrupts();
	HyperV::EOI();

	m_scheduler->Schedule();
	x64_restore_flags(flags);
}

void Kernel::CreateThread(ThreadStart start, void* arg)
{
	//Current thread
	KernelThread* current = GetCurrentThread();
	Print("CreateThread - Id: 0x%x Process: 0x%016x\n", current->Id, current->Process);
	
	KernelThread* thread = new KernelThread();
	memset(thread, 0, sizeof(KernelThread));
	thread->Process = current->Process;
	thread->Id = ++m_lastId;
	thread->State == ThreadState::Ready;
	thread->Start = start;
	thread->Arg = arg;
	thread->TEB = new ThreadEnvironmentBlock();
	thread->TEB->SelfPointer = thread->TEB;
	thread->TEB->ThreadId = thread->Id;
	m_threads->insert({ thread->Id, thread });

	//Create thread stack, point to bottom (stack grows shallow)
	uintptr_t stack = (uintptr_t)this->AllocatePage(0, KERNEL_THREAD_STACK_SIZE, MemoryProtection::PageReadWrite);
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
	Print("Kernel::ThreadInitThunk\n");
	KernelThread* current = kernel.GetCurrentThread();
	Print("Start: 0x%016x, Arg: 0x%016x\n", current->Start, current->Arg);
	
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
	Assert(x64_ReadGS() != 0);
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

void* Kernel::Allocate(const size_t size, const uintptr_t callerAddress)
{
	return m_heap->Allocate(size, callerAddress);
}

void Kernel::Deallocate(void* address)
{
	m_heap->Deallocate(address);
}

void* Kernel::AllocatePage(uintptr_t address, const size_t count, const MemoryProtection& protection)
{
	return m_virtualMemory->Allocate(address, count, protection, *m_addressSpace);
}

Handle Kernel::CreateSemaphore(const size_t initial, const size_t maximum, const std::string& name)
{
	KSemaphore* semaphore = new KSemaphore(initial, maximum, name);
	return semaphore;
}

bool Kernel::ReleaseSemaphore(Handle handle, const size_t releaseCount)
{
	KSemaphore* semaphore = GetSemaphore(handle);

	if (m_scheduler->Enabled)
	{
		cpu_flags_t flags = x64_disable_interrupts();

		m_scheduler->SemaphoreRelease(semaphore, releaseCount);

		x64_restore_flags(flags);
	}
	else
	{
		semaphore->Signal(releaseCount);
	}
	
	return true;
}

bool Kernel::CloseSemaphore(Handle handle)
{
	KSemaphore* semaphore = GetSemaphore(handle);
	semaphore->DecRefCount();

	if (semaphore->IsClosed())
		delete semaphore;

	return true;
}

WaitStatus Kernel::WaitForSemaphore(Handle handle, size_t timeoutMs, size_t units)
{
	KSemaphore* semaphore = GetSemaphore(handle);

	if (m_scheduler->Enabled)
	{
		cpu_flags_t flags = x64_disable_interrupts();

		const nano100_t timeout = timeoutMs * 1000000 / 100;
		WaitStatus status = m_scheduler->SemaphoreWait(semaphore, timeout);

		x64_restore_flags(flags);
		return status;
	}
	else
	{
		semaphore->Wait(units, timeoutMs);
		return WaitStatus::Signaled;
	}
}

void Kernel::RegisterInterrupt(const InterruptVector interrupt, const InterruptContext& context)
{
	Assert(m_interruptHandlers->find(interrupt) == m_interruptHandlers->end());
	m_interruptHandlers->insert({ interrupt, context });
}

paddr_t Kernel::AllocatePhysical(const size_t count)
{
	paddr_t address;
	Assert(m_pfnDb->AllocateContiguous(address, count));
	return address;
}

void* Kernel::VirtualMap(const void* address, const std::vector<paddr_t>& addresses, const MemoryProtection& protection)
{
	return m_virtualMemory->VirtualMap((uintptr_t)address, addresses, protection, *m_addressSpace);
}

Device* Kernel::GetDevice(const std::string path)
{
	return m_deviceTree.GetDevice(path);
}

Handle Kernel::CreateFile(const char* path, GenericAccess access)
{
	Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver(); //TODO: figure out why HardDriveDriver didnt work

	FileHandle* handle = hdd->OpenFile(path, access);
	return handle;
}

bool Kernel::ReadFile(Handle handle, void* buffer, size_t bufferSize, size_t* bytesRead)
{
	Assert(handle);
	FileHandle* file = (FileHandle*)handle;

	Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver(); //TODO: figure out why HardDriveDriver didnt work
	
	*bytesRead = hdd->ReadFile(file, buffer, bufferSize);

	return true;
}

bool Kernel::SetFilePosition(Handle handle, size_t position)
{
	Assert(handle);
	
	FileHandle* file = (FileHandle*)handle;
	Assert(position < file->Length);
	file->Position = position;

	return true;
}

bool Kernel::CreateProcess(const char* path)
{
	Print("CreateProcess %s\n", path);
	Handle file = CreateFile(path, GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	Assert(SetFilePosition(file, dosHeader.e_lfanew));
	Assert(ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		Assert(false);
	}
		
	KernelProcess* process = new KernelProcess();
	memset(process, 0, sizeof(KernelProcess));
	strcpy(process->Name, path);
	process->Id = ++m_lastId;
	process->VirtualAddress = new UserAddressSpace();
	Assert(m_pagePool->AllocatePage(&process->CR3));
	PageTables* pt = new PageTables(process->CR3);
	pt->SetPool(m_pagePool);
	pt->LoadKernelMappings(m_pageTables);

	//Swap to new process address space
	__writecr3(process->CR3);
	Print("NewCr3: 0x%016x\n", process->CR3);

	//Allocate pages
	const size_t pageCount = SIZE_TO_PAGES(peHeader.OptionalHeader.SizeOfImage);
	void* address = m_virtualMemory->Allocate(peHeader.OptionalHeader.ImageBase, pageCount, MemoryProtection::PageReadWriteExecute, *process->VirtualAddress);//TODO: protection
	Assert(address);

	//Read headers
	Assert(SetFilePosition(file, 0));
	Assert(ReadFile(file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	Assert(read == peHeader.OptionalHeader.SizeOfHeaders);

	PIMAGE_NT_HEADERS64 pNtHeader = MakePtr(PIMAGE_NT_HEADERS64, address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)address + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			Assert(SetFilePosition(file, section[i].PointerToRawData));
			Assert(ReadFile(file, (void*)destination, rawSize, &read));
			Assert(read == section[i].SizeOfRawData);
		}
	}

	//Create thread
	KernelThread* thread = new KernelThread();
	memset(thread, 0, sizeof(KernelThread));
	thread->Process = process;
	thread->Id = ++m_lastId;
	thread->State == ThreadState::Ready;
	thread->Start = (ThreadStart)peHeader.OptionalHeader.AddressOfEntryPoint;
	thread->Arg = address;
	thread->TEB = new ThreadEnvironmentBlock();
	thread->TEB->SelfPointer = thread->TEB;
	thread->TEB->ThreadId = thread->Id;
	m_threads->insert({ thread->Id, thread });

	//Create thread stack, point to bottom (stack grows shallow)
	uintptr_t stack = (uintptr_t)m_virtualMemory->Allocate(0, KERNEL_THREAD_STACK_SIZE << PAGE_SHIFT, MemoryProtection::PageReadWriteExecute, *process->VirtualAddress);
	stack += (KERNEL_THREAD_STACK_SIZE << PAGE_SHIFT);//End of stack

	//Subtract paramater area
	stack -= 32;

	//Allocate space for context
	thread->Context = new uint8_t[x64_CONTEXT_SIZE];
	x64_init_context(thread->Context, (void*)stack, &Kernel::ThreadInitThunk);

	//Add to scheduler
	m_scheduler->Add(*thread);

	return false;
}

void* Kernel::VirtualAlloc(void* address, size_t size, MemoryAllocationType allocationType, MemoryProtection protect)
{
	return nullptr;
}

