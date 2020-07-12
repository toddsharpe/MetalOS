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
#include "KThread.h"

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
	m_scheduler(),

	m_hyperV(),

	m_deviceTree(),

	m_timer(),

	m_pdb()
{

}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { this->Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
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
	__writecr3(m_pageTables->GetCr3());

	Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x\n", m_physicalAddress, m_imageSize);
	Print("  PhysicalAddressSize: 0x%16x\n", m_memoryMap->GetPhysicalAddressSize());

	//Enable WRGSBASE instruction
	__writecr4(__readcr4() | (1 << 16));
	Print("  CR3: 0x%16x CR4: 0x%16x\n", __readcr3(), __readcr4());

#define IA32_STAR_MSR 0xC0000081 //IA32_STAR_REG
#define IA32_LSTAR_MSR 0xC0000082 //Target RIP
#define IA32_FMASK_MSR 0xC0000084 //IA32_FMASK_REG
#define IA32_EFER_MSR 0xC0000080
	Print("  STAR: 0x%16x LSTAR: 0x%16x EFER: 0x%016x\n",
		__readmsr(IA32_STAR_MSR), __readmsr(IA32_STAR_MSR), __readmsr(IA32_EFER_MSR));

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

	//Test interrupts
	__debugbreak();

	//Create boot thread
	KThread* bootThread = new KThread(nullptr, nullptr, (void*)new uint8_t[x64_CONTEXT_SIZE]);

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
	m_timer->SetPeriodic(SECOND / 64, (uint8_t)InterruptVector::Timer0);

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
	{
		m_textScreen->Printf("ISR: 0x%x, Code: %d, RBP: 0x%016x, RIP: 0x%016x, RSP: 0x%016x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
		m_textScreen->Printf("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
		m_textScreen->Printf("   CS: 0x%16x,  SS: 0x%16x,  GS: 0x%16x,  FS: 0x%16x\n", pFrame->CS, pFrame->SS, x64_ReadGS(), x64_ReadFS());
		m_textScreen->Printf("   Current CS: 0x%16x,  SS: 0x%16x CR3: 0x%016x\n", x64_ReadCS(), x64_ReadSS(), __readcr3());

		x64::PrintGDT();
		__halt();
	}

	const auto& it = m_interruptHandlers->find(vector);
	if (it != m_interruptHandlers->end())
	{
		InterruptContext ctx = it->second;
		ctx.Handler(ctx.Context);
		return;
	}
	
	m_textScreen->Printf("ISR: 0x%x, Code: %x, RBP: 0x%16x, RIP: 0x%16x, RSP: 0x%16x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	m_textScreen->Printf("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	m_textScreen->Printf("   CS: 0x%16x,  SS: 0x%16x,  GS: 0x%16x,  FS: 0x%16x\n", pFrame->CS, pFrame->SS, x64_ReadGS(), x64_ReadFS());
	m_textScreen->Printf("   Current CS: 0x%16x,  SS: 0x%16x CR3: 0x%016x\n", x64_ReadCS(), x64_ReadSS(), __readcr3());
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

	x64_swapgs();

	KThread* current = m_scheduler->GetCurrentThread();
	current->Display();

	UserThread* user = m_scheduler->GetCurrentThread()->GetUserThread();
	uintptr_t base = user == nullptr ? KernelBaseAddress : user->GetProcess().GetImageBase();
	Print("base: 0x%016x\n", base);

	StackWalk sw(&context, base);

	while (sw.HasNext())
	{
		Print("IP: 0x%016x ", context.Rip);
		if (m_pdb != nullptr && (context.Rip >= KernelBaseAddress))
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
		if (m_pdb != nullptr && (context.Rip >= KernelBaseAddress))
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

void Kernel::CreateKernelThread(ThreadStart start, void* arg)
{
	//Current thread
	KThread* current = m_scheduler->GetCurrentThread();
	Print("CreateThread - Id: 0x%x\n", current->GetId());
	
	//Create thread stack, point to bottom (stack grows shallow on x86)
	uintptr_t stack = (uintptr_t)this->AllocateKernelPage(0, KThread::StackPageCount, MemoryProtection::PageReadWrite);
	stack += (KThread::StackPageCount << PAGE_SHIFT);

	//Subtract paramater area
	stack -= 32;

	//Create thread context
	void* context = new uint8_t[x64_CONTEXT_SIZE];
	x64_init_context(context, (void*)stack, &Kernel::KernelThreadInitThunk);

	//Add kernel thread
	KThread* thread = new KThread(start, arg, context);
	m_scheduler->AddReady(*thread);
}

void Kernel::KernelThreadInitThunk()
{
	Print("Kernel::KernelThreadInitThunk\n");
	KThread* current = kernel.m_scheduler->GetCurrentThread();
	current->Display();
	
	//Run thread
	//current->Run(); - leads to page fault, stack pointer was wrong but why?
	current->m_start(current->m_arg);

	Print("Kill thread: %d\n", current->GetId());

	//Exit thread TODO
	__halt();
}

//this should use sysret
void Kernel::UserThreadInitThunk()
{
	Print("Kernel::UserThreadInitThunk\n");
	KThread* current = kernel.m_scheduler->GetCurrentThread();
	current->Display();

	UserThread* user = current->GetUserThread();
	user->DisplayDetails();
	
#define IA32_STAR_MSR 0xC0000081 //IA32_STAR_REG
#define IA32_LSTAR_MSR 0xC0000082 //Target RIP
#define IA32_FMASK_MSR 0xC0000084 //IA32_FMASK_REG
#define IA32_EFER_MSR 0xC0000080
	Print("  STAR: 0x%16x LSTAR: 0x%16x EFER: 0x%016x\n",
		__readmsr(IA32_STAR_MSR), __readmsr(IA32_LSTAR_MSR), __readmsr(IA32_EFER_MSR));

	//Start
	x64::SetKernelInterruptStack(current->GetStack());
	x64_start_user_thread(user->GetContext(), user->GetTEB());
}

void Kernel::Sleep(nano_t value)
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

Handle Kernel::CreateFile(const std::string& path, GenericAccess access)
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

bool Kernel::CreateProcess(const std::string& path)
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
	
	//Create new process to create new address space
	UserProcess* process = new UserProcess(path);

	//Swap to new process address space to load file
	//TODO: page into kernel, load, then remove and page into process so we dont switch address space
	__writecr3(process->GetCR3());
	Print("NewCr3: 0x%016x\n", process->GetCR3());

	//Allocate pages
	void* address = VirtualAlloc(*process, (void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWriteExecute);
	Assert(address);

	//Init in context of address space
	process->Init(address);

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

	//Save init pointers
	process->InitProcess = (void*)Loader::GetProcAddress(address, "InitProcess");
	process->InitThread = (void*)Loader::GetProcAddress(address, "InitThread");
	Print("Proc: 0x%016x Thread: 0x%016x\n", process->InitProcess, process->InitThread);

	//Load KernelAPI
	Handle k = Loader::LoadLibrary(*process, "mosapi.dll");
	Print("mosapi loaded at 0x%016x\n", k);

	//Patch imports of process for just mosapi
	Loader::KernelExports(address, k);

	//Create thread TODO: reserve vs commit stack size
	CreateThread(*process, pNtHeader->OptionalHeader.SizeOfStackReserve, nullptr, nullptr, process->InitProcess);
	return true;
}

//TODO - usermode thread init thunk (it will bring us to ring 3 using iret and start address)

Handle Kernel::CreateThread(UserProcess& process, size_t stackSize, ThreadStart startAddress, void* arg, void* entry)
{
	//TODO: validate addresses
	
	//Allocate user stack
	void* stack = VirtualAlloc(process, nullptr, stackSize, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWrite);
	void* stackTop = MakePtr(void*, stack, PAGE_ALIGN(stackSize) - 32);//Register parameter area
	Print("User Stack: 0x%016x->0x%016x\n", stackTop, stack);

	//Create user thread
	UserThread* userThread = new UserThread(startAddress, arg, stackTop, entry, process);

	//Create kernel thread stack, point to bottom (stack grows shallow on x86)
	uintptr_t kStack = (uintptr_t)this->AllocateKernelPage(0, KThread::StackPageCount, MemoryProtection::PageReadWrite);
	kStack += (KThread::StackPageCount << PAGE_SHIFT) - 32;
	Print("Kernel Stack: 0x%016x->0x%016x\n", kStack, kStack - (KThread::StackPageCount << PAGE_SHIFT));

	//Create kernel thread context
	void* context = new uint8_t[x64_CONTEXT_SIZE];
	x64_init_context(context, (void*)kStack, Kernel::UserThreadInitThunk);

	//Create kernel thread
	KThread* thread = new KThread(nullptr, nullptr, context, userThread);
	process.AddThread(*thread);
	m_scheduler->AddReady(*thread);
	
	return userThread;
}

void* Kernel::VirtualAlloc(UserProcess& process, void* address, size_t size, MemoryAllocationType allocationType, MemoryProtection protect)
{
	void* allocated = m_virtualMemory->Allocate((uintptr_t)address, SIZE_TO_PAGES(size), protect, process.GetAddressSpace());
	Assert(allocated);
	return allocated;
}

uint64_t Kernel::Syscall(SystemcallFrame* frame)
{
	//Print("Call: 0x%016x UserIP: 0x%016x\n", frame->SystemCall, frame->UserIP);
	//Print("Arg0: 0x%016x Arg1: 0x%016x Arg2: 0x%016x Arg3: 0x%016x\n", frame->Arg0, frame->Arg1, frame->Arg2, frame->Arg3);
	
	//Swap to kernel TEB
	x64_swapgs();

	//Show thread
	//KThread* current = m_scheduler->GetCurrentThread();
	//current->Display();

	uint32_t ret = -1;
	//Do systemcall
	switch (frame->SystemCall)
	{
	case SystemCall::GetSystemInfo:
		ret = GetSystemInfo((SystemInfo*)frame->Arg0);
		break;

	case SystemCall::DebugPrint:
		ret = DebugPrint((char*)frame->Arg0);
		break;

	case SystemCall::ExitProcess:
		ret = ExitProcess(frame->Arg0);
		break;

	case SystemCall::CreateWindow:
		ret = CreateWindow((char*)frame->Arg0);
		break;

	case SystemCall::GetWindowRect:
		ret = GetWindowRect((Handle)frame->Arg0, (Rectangle*)frame->Arg1);
		break;

	case SystemCall::GetMessage:
		ret = GetMessage((Message*)frame->Arg0);
		break;

	default:
		Assert(false);
		break;
	}

	//Swap back to user TEB
	x64_swapgs();

	return ret;
}

void Kernel::PostMessage(Message* msg)
{
	if (!Window)
	{
		Print("No window!\n");
		return;
	}

	Window->PostMessage(msg);
}
