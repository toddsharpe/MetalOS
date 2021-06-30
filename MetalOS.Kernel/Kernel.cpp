#include "Kernel.h"

typedef EFI_GUID GUID;
#define PACKED
#include <PlatformAcpi.h>
#include <intrin.h>
#include <cstdarg>
#include "System.h"
#include "Main.h"
#include <functional>
#include "AcpiDevice.h"
#include "UartDriver.h"
#include "RuntimeSupport.h"
#include "LoadingScreen.h"
#include "HyperVTimer.h"
#include "HyperV.h"
#include "KernelHeap.h"
#include "BootHeap.h"
#include "StackWalk.h"
#include "KSemaphore.h"
#include "RamDriveDriver.h"
#include "SoftwareDevice.h"
#include "IoApicDriver.h"
#include "Loader.h"
#include <string>
#include "KThread.h"
#include <MetalOS.Arch.h>

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
	m_scheduler(),

	m_hyperV(),

	m_deviceTree(),

	m_timer(),

	m_pdb(),

	m_debugger()
{

}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { this->Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define Trace() Print(__FILE__ "-" STR(__LINE__));
void Kernel::Initialize(const PLOADER_PARAMS params)
{
	ArchInitialize();
	
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
	m_display->ColorScreen(Colors::Black);
	m_textScreen = new TextScreen(*m_display);
	m_printer = m_textScreen;

	//Initialize and process memory map
	m_memoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, params->MemoryMap);
	m_memoryMap->ReclaimBootPages();
	m_memoryMap->MergeConventionalPages();

	//UEFI configuration tables (saves them to boot heap)
	m_configTables = new ConfigTables(params->ConfigTables, params->ConfigTableSizes);

	//Initialize page table pool
	PageTables::Pool = new PageTablesPool(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);

	//Construct kernel page table
	m_pageTables = new PageTables();
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
	ArchSetPagingRoot(m_pageTables->GetCr3());

	Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x\n", m_physicalAddress, m_imageSize);
	Print("  PhysicalAddressSize: 0x%16x\n", m_memoryMap->GetPhysicalAddressSize());

	//Check if SSE
	int r[4];
	__cpuid(r, 0x1);
	Assert(r[3] & (1 << 25));//sse

	__writecr0(__readcr0() & ~(1 << 2));//!Emulated
	__writecr0(__readcr0() | (1 << 1));
	__writecr4(__readcr4() | (1 << 9));//OSFXSR 
	__writecr4(__readcr4() | (1 << 10));//OSXMMEXCPT 

	//Check if OSXsave is enabled
	if (!(r[2] & (1 << 27)))
	{
		//osxsave not enabled, set it

		//Have to have xave
		Assert(r[3] & (1 << 26));//xsave

		//Enable osxsave
		__writecr4(__readcr4() | (1 << 18));//OSXSAVE
		Assert(__readcr4() & (1 << 18));//Check osxsave enabled
	}

	//Check if AVX is supported
	if (r[2] & (1 << 28));
	{
		//Enable avx
		_xsetbv(0, _xgetbv(0) | 0x7);

		Print("Enabled AVX\n");
	}

	if (r[2] & (1 << 20))
		Print("SSE 4.2\n");
	if (r[2] & (1 << 19))
		Print("SSE 4.1\n");
	if (r[2] & (1 << 9))
		Print("SSSE 3\n");
	if (r[2] & (1 << 0))
		Print("SSE 3\n");
	if (r[3] & (1 << 26))
		Print("SSE 2\n");

	//Check AVX512
	__cpuidex(r, 0x7, 0);
	Print("EBX: 0x%08x\n", r[1]);
	if (r[1] & (1 << 16))
		Print("AVX 512\n");

	//Test UEFI runtime access
	EFI_TIME time = { 0 };
	m_runtime.GetTime(&time, nullptr);
	Print("  Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Initialize virtual memory
	m_pfnDb = new PhysicalMemoryManager(*m_memoryMap);
	Assert(m_pfnDb->GetSize() == pfnDbSize);
	m_virtualMemory = new VirtualMemoryManager(*m_pfnDb);
	m_addressSpace = new VirtualAddressSpace(KernelRuntimeStart, KernelRuntimeEnd, true);

	//Initialize Heap
	m_heap = new KernelHeap(*m_virtualMemory, *m_addressSpace);
	m_heapInitialized = true;

	//PDB
	m_pdb = new Pdb(KernelPdbStart);

	//Interrupts
	m_interruptHandlers = new std::map<InterruptVector, InterruptContext>();
	m_interruptHandlers->insert({ InterruptVector::Timer0, { &Kernel::OnTimer0, this} });
	m_interruptHandlers->insert({ InterruptVector::Breakpoint, { [](void* arg) { ((Kernel*)arg)->Printf("Debug Breakpoint Exception\n"); }, this} });

	//Test interrupts
	__debugbreak();

	//Create boot thread
	KThread* bootThread = new KThread(nullptr, nullptr);

	//Process and thread containers
	m_processes = new std::map<uint32_t, UserProcess*>();
	m_scheduler = new Scheduler(*bootThread);

	//Create idle thread
	CreateKernelThread(&Kernel::IdleThread, this);

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
	m_timer->SetPeriodic(SECOND / 128, (uint8_t)InterruptVector::Timer0);

	//Attach drivers and enumerate tree
	m_deviceTree.EnumerateChildren();
	m_deviceTree.Display();

	//Show loading screen (works but we don't need yet)
	//LoadingScreen loadingScreen(*m_display);
	//loadingScreen.Initialize();

	//Install interrupts
	//Device* com2;
	//Assert(m_deviceTree.GetDeviceByName("COM2", &com2));
	//UartDriver* com2Driver = ((UartDriver*)com2->GetDriver());
	//m_interruptHandlers->insert({ InterruptVector::COM2, { &UartDriver::OnInterrupt, com2Driver} });

	//Device* ioapic;
	//Assert(m_deviceTree.GetDeviceByName("IOAPIC", &ioapic));
	//IoApicDriver* ioapicDriver = ((IoApicDriver*)ioapic->GetDriver());
	//ioapicDriver->MapInterrupt(InterruptVector::COM2, 3);//TODO: find a way to get 3 from ACPI
	//ioapicDriver->UnmaskInterrupt(3);

	//Debugger
	m_debugger = new Debugger();
	m_debugger->Initialize();

	//Done
	Print("Kernel Initialized\n");
}

void Kernel::HandleInterrupt(InterruptVector vector, PINTERRUPT_FRAME pFrame)
{
	//if (vector == InterruptVector::DoubleFault)
	//{
	//	m_textScreen->Printf("ISR: 0x%x, Code: %d, RBP: 0x%016x, RIP: 0x%016x, RSP: 0x%016x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	//	m_textScreen->Printf("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	//	m_textScreen->Printf("   CS: 0x%16x,  SS: 0x%16x,  GS: 0x%16x\n", pFrame->CS, pFrame->SS, x64_ReadGS());
	//	m_textScreen->Printf("   Current CS: 0x%16x,  SS: 0x%16x CR3: 0x%016x\n", x64_ReadCS(), x64_ReadSS(), __readcr3());

	//	x64::PrintGDT();
	//	ArchWait();
	//}

	const auto& it = m_interruptHandlers->find(vector);
	if (it != m_interruptHandlers->end())
	{
		InterruptContext ctx = it->second;
		ctx.Handler(ctx.Context);
		HyperV::EOI();
		return;
	}
	
	m_textScreen->Printf("ISR: 0x%x, Code: %x, RBP: 0x%16x, RIP: 0x%16x, RSP: 0x%16x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	m_textScreen->Printf("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	//m_textScreen->Printf("   CS: 0x%16x,  SS: 0x%16x,  GS: 0x%16x\n", pFrame->CS, pFrame->SS, x64_ReadGS());
	//m_textScreen->Printf("   Current CS: 0x%16x,  SS: 0x%16x CR3: 0x%016x\n", x64_ReadCS(), x64_ReadSS(), __readcr3());
	switch (vector)
	{
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


	KThread* current = m_scheduler->GetCurrentThread();
	current->Display();

	UserThread* user = m_scheduler->GetCurrentThread()->GetUserThread();

	StackWalk sw(&context);
	while (sw.HasNext())
	{
		uintptr_t base = user == nullptr ? KernelBaseAddress : user->GetProcess().GetModuleBase(context.Rip);
		Print("IP: 0x%016x Base: 0x%016x", context.Rip, base);
		if (m_pdb != nullptr && (context.Rip >= KernelBaseAddress))
			m_pdb->PrintStack((uint32_t)context.Rip - KernelBaseAddress);
		else
			Print("\n");

		sw.Next(base);
	}

	while (true)
		ArchWait();
}

bool inBugcheck = false;
void Kernel::Bugcheck(const char* file, const char* line, const char* assert)
{
	cpu_flags_t flags = ArchDisableInterrupts();
	
	if (inBugcheck)
	{
		m_textScreen->Printf("%s\n%s\n%s\n", file, line, assert);

		while(true)
			ArchWait();
	}
	
	inBugcheck = true;
	
	if (m_scheduler != nullptr)
		m_scheduler->Enabled = false;

	if (m_timer != nullptr)
		m_timer->Disable();

	m_textScreen->Printf("%s\n%s\n%s\n", file, line, assert);

	if (m_scheduler)
	{
		KThread* thread = m_scheduler->GetCurrentThread();
		thread->Display();
	}

	//TODO: have arch library do this

	uint8_t buffer[sizeof(x64_context)];
	//Assert(sizeof(x64_context) == x64_CONTEXT_SIZE);
	ArchSaveContext(buffer);

	x64_context* x64context = (x64_context*)buffer;

	CONTEXT context = { 0 };
	context.Rip = x64context->Rip;
	context.Rsp = x64context->Rsp;
	context.Rbp = x64context->Rbp;

	StackWalk sw(&context);
	uint32_t rva = (uint32_t)context.Rip - KernelBaseAddress;
	
	while (sw.HasNext())
	{
		Print("IP: 0x%016x ", context.Rip);
		if (m_pdb != nullptr && (context.Rip >= KernelBaseAddress))
			m_pdb->PrintStack((uint32_t)context.Rip - KernelBaseAddress);
		else
			Print("\n");

		sw.Next(KernelBaseAddress);
	}

	//m_display->ColorScreen(Red);

	//m_textScreen->ResetX();
	//m_textScreen->ResetY();

	while (true)
		ArchWait();
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
		ArchWait();
	}

	Status = AcpiInitializeTables(nullptr, 16, FALSE);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeTables: %d\n", Status);
		ArchWait();
	}

	//TODO: notify handlers

	/* Install the default address space handlers. */
	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_MEMORY, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not initialise SystemMemory handler, %s!", AcpiFormatException(Status));
		ArchWait();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_IO, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not initialise SystemIO handler, %s!", AcpiFormatException(Status));
		ArchWait();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_PCI_CONFIG, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not initialise PciConfig handler, %s!", AcpiFormatException(Status));
		ArchWait();
	}

	Status = AcpiLoadTables();
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiLoadTables: %d\n", Status);
		ArchWait();
	}

	//Local handlers should be installed here

	Status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiEnableSubsystem: %d\n", Status);
		ArchWait();
	}

	Status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeObjects: %d\n", Status);
		ArchWait();
	}

	Print("ACPI Finished\n");
}

void Kernel::OnTimer0()
{
	m_scheduler->Schedule();
}

KThread* Kernel::CreateKernelThread(ThreadStart start, void* arg)
{
	//Current thread
	KThread* current = m_scheduler->GetCurrentThread();
	Print("CreateThread - Id: 0x%x\n", current->GetId());
	
	//Add kernel thread
	KThread* thread = new KThread(start, arg);
	thread->InitContext(&Kernel::KernelThreadInitThunk);
	m_scheduler->AddReady(*thread);
	return thread;
}

void Kernel::ExitKernelThread()
{
	Print("Kernel::ExitKernelThread\n");

	m_scheduler->KillThread();
}

Handle Kernel::LoadKernelLibrary(const char* path)
{
	return Loader::LoadKernelLibrary(path);
}

void Kernel::KernelThreadInitThunk()
{
	Print("Kernel::KernelThreadInitThunk\n");
	KThread* current = kernel.m_scheduler->GetCurrentThread();
	current->Display();
	
	//Run thread
	current->Run();
	Print("Kill thread: %d\n", current->GetId());

	//Exit thread TODO
	kernel.ExitKernelThread();
	ArchWait();
}

void Kernel::UserThreadInitThunk()
{
	Print("Kernel::UserThreadInitThunk\n");
	KThread* current = kernel.m_scheduler->GetCurrentThread();
	current->Display();

	UserThread* user = current->GetUserThread();
	user->DisplayDetails();

	user->GetProcess().Display();
	user->GetProcess().DisplayDetails();
	ArchSetInterruptStack(current->GetStackPointer());
	user->Run();
}

void Kernel::KernelThreadSleep(nano_t value)
{
	m_scheduler->Sleep(value);
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

void* Kernel::AllocateKernelPage(uintptr_t address, const size_t count, const MemoryProtection& protection)
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
		cpu_flags_t flags = ArchDisableInterrupts();

		m_scheduler->SemaphoreRelease(semaphore, releaseCount);

		ArchRestoreFlags(flags);
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
		cpu_flags_t flags = ArchDisableInterrupts();

		const nano100_t timeout = timeoutMs * 1000000 / 100;
		WaitStatus status = m_scheduler->SemaphoreWait(semaphore, timeout);

		ArchRestoreFlags(flags);
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

FileHandle* Kernel::CreateFile(const std::string& path, GenericAccess access)
{
	Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver(); //TODO: figure out why HardDriveDriver didnt work

	FileHandle* handle = hdd->OpenFile(path, access);
	return handle;
}

bool Kernel::ReadFile(FileHandle* file, void* buffer, size_t bufferSize, size_t* bytesRead)
{
	Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver(); //TODO: figure out why HardDriveDriver didnt work
	
	*bytesRead = hdd->ReadFile(file, buffer, bufferSize);
	return true;
}

bool Kernel::SetFilePosition(FileHandle* file, size_t position)
{
	if (position >= file->Length)
		return false;

	file->Position = position;
	return true;
}

void Kernel::CloseFile(FileHandle* file)
{
	delete file;
}

bool Kernel::CreateProcess(const std::string& path)
{
	Print("CreateProcess %s\n", path);
	FileHandle* file = CreateFile(path, GenericAccess::Read);
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
	
	//Create new process to create new address space
	UserProcess* process = new UserProcess(path);

	//Swap to new process address space to load file
	//TODO: page into kernel, load, then remove and page into process so we dont switch address space
	ArchSetPagingRoot(process->GetCR3());
	Print("NewCr3: 0x%016x\n", process->GetCR3());

	//Allocate pages
	void* address = VirtualAlloc(*process, (void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWriteExecute);
	Assert(address);

	//Init in context of address space
	process->Init(address);
	process->AddModule(path.c_str(), address);

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

	//Load KernelAPI
	Handle api = Loader::LoadLibrary(*process, "mosrt.dll");
	Print("mosrt loaded at 0x%016x\n", api);

	//Save init pointers in library
	process->InitProcess = (void*)Loader::GetProcAddress(api, "InitProcess");
	process->InitThread = (void*)Loader::GetProcAddress(api, "InitThread");
	Print("Proc: 0x%016x Thread: 0x%016x\n", process->InitProcess, process->InitThread);

	//Patch imports of process for just mosapi
	Loader::KernelExports(address, api);

	//Create thread TODO: reserve vs commit stack size
	CreateThread(*process, pNtHeader->OptionalHeader.SizeOfStackReserve, nullptr, nullptr, process->InitProcess);
	return true;
}

KThread* Kernel::CreateThread(UserProcess& process, size_t stackSize, ThreadStart startAddress, void* arg, void* entry)
{
	//TODO: validate addresses
	
	//Create user thread
	UserThread* userThread = new UserThread(startAddress, arg, entry, stackSize, process);

	//Create kernel thread
	KThread* thread = new KThread(nullptr, nullptr, userThread);
	thread->InitContext(&Kernel::UserThreadInitThunk);
	process.AddThread(*thread);
	m_scheduler->AddReady(*thread);
	
	return thread;
}

void* Kernel::VirtualAlloc(UserProcess& process, void* address, size_t size, MemoryAllocationType allocationType, MemoryProtection protect)
{
	void* allocated = m_virtualMemory->Allocate((uintptr_t)address, SIZE_TO_PAGES(size), protect, process.GetAddressSpace());
	Assert(allocated);
	return allocated;
}

void Kernel::PostMessage(Message* msg)
{
	if (!Window)
		return;

	Window->PostMessage(msg);
}

bool Kernel::IsValidUserPointer(const void* p)
{
	//Make sure pointer is in User's address half
	if ((uintptr_t)p > UserStop)
		return false;

	//Make sure pointer is User's address space
	UserProcess& process = m_scheduler->GetCurrentProcess();
	return process.GetAddressSpace().IsValidPointer(p);
}
