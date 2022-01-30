#include "Kernel.h"
#include "Assert.h"

#define PACKED
#include <tianocore-edk2/IndustryStandard/PlatformAcpi.h>
#include <intrin.h>
#include <cstdarg>
#include "System.h"
#include "Main.h"
#include <functional>
#include "AcpiDevice.h"
#include "UartDriver.h"
#include "RuntimeSupport.h"
#include "PortableExecutable.h"
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
#include <Path.h>

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
	
	m_librarySpace(),
	m_pdbSpace(),
	m_stackSpace(),
	m_heapSpace(),
	m_runtimeSpace(),
	m_windowsSpace(),

	m_heapInitialized(),
	m_pfnDbAddress(),
	m_pfnDb(),
	m_virtualMemory(),
	m_heap(),

	m_interruptHandlers(),

	m_processes(),
	m_scheduler(),
	m_objectsRingBuffers(),

	m_modules(),

	m_hyperV(),

	m_deviceTree(),

	m_timer(),

	m_pdb(),

	m_windows(),

	m_debugger()
{

}

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
	m_display = new Display(params->Display, KernelGraphicsDevice);
	m_display->ColorScreen(Colors::Black);
	m_textScreen = new TextScreen(*m_display);
	m_printer = m_textScreen;

	//Initialize and process memory map
	m_memoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, *params->MemoryMap);
	m_memoryMap->ReclaimBootPages();
	m_memoryMap->MergeConventionalPages();

	//UEFI configuration tables (saves them to boot heap)
	m_configTables = new ConfigTables(params->ConfigTables, params->ConfigTableSizes);

	//Initialize page table pool
	PageTables::Pool = new PageTablesPool(KernelPageTablesPool, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);

	//Construct kernel page table
	m_pageTables = new PageTables();
	m_pageTables->MapKernelPages(KernelBaseAddress, params->KernelAddress, EFI_SIZE_TO_PAGES(params->KernelImageSize));
	m_pageTables->MapKernelPages(KernelPageTablesPool, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	m_pageTables->MapKernelPages(KernelGraphicsDevice, params->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(params->Display.FrameBufferSize));
	m_pageTables->MapKernelPages(KernelPfnDbStart, params->PfnDbAddress, EFI_SIZE_TO_PAGES(params->PfnDbSize));
	m_pageTables->MapKernelPages(KernelKernelPdb, params->PdbAddress, EFI_SIZE_TO_PAGES(params->PdbSize));

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

	//Test UEFI runtime access
	EFI_TIME time = { 0 };
	m_runtime.GetTime(&time, nullptr);
	Print("  Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Initialize virtual memory
	m_pfnDb = new PhysicalMemoryManager(*m_memoryMap);
	Assert(m_pfnDb->GetSize() == pfnDbSize);
	m_virtualMemory = new VirtualMemoryManager(*m_pfnDb);

	//Initialize address spaces (TODO: condense with split method)
	m_librarySpace = new VirtualAddressSpace(KernelLibraryStart, KernelLibraryEnd, true);
	m_pdbSpace = new VirtualAddressSpace(KernelPdbStart, KernelPdbEnd, true);
	m_stackSpace = new VirtualAddressSpace(KernelStackStart, KernelStackEnd, true);
	m_heapSpace = new VirtualAddressSpace(KernelHeapStart, KernelHeapEnd, true);
	m_runtimeSpace = new VirtualAddressSpace(KernelRuntimeStart, KernelRuntimeEnd, true);
	m_windowsSpace = new VirtualAddressSpace(KernelWindowsStart, KernelWindowsEnd, true);

	//Initialize Heap
	m_heap = new KernelHeap(*m_virtualMemory, *m_heapSpace);
	m_heapInitialized = true;

	//PDB
	m_pdb = new Pdb((void*)KernelKernelPdb, (Handle*)KernelBaseAddress);

	//Interrupts
	m_interruptHandlers = new std::map<InterruptVector, InterruptContext>();
	m_interruptHandlers->insert({ InterruptVector::Timer0, { &Kernel::OnTimer0, this} });
	m_interruptHandlers->insert({ InterruptVector::Breakpoint, { [](void* arg) { ((Kernel*)arg)->Printf("Debug Breakpoint Exception\n"); }, this} });

	//Test interrupts
	__debugbreak();

	//Create boot thread
	KThread* bootThread = new KThread(nullptr, nullptr);
	bootThread->Name = "boot";

	//Process and thread containers
	m_processes = new std::map<uint32_t, UserProcess*>();
	m_scheduler = new Scheduler(*bootThread);
	m_objectsRingBuffers = new std::map<std::string, UserRingBuffer*>();

	//Modules
	m_modules = new std::list<KeLibrary>();
	KeLibrary keLibrary = { "moskrnl.exe", (Handle)KernelBaseAddress, m_pdb };
	m_modules->push_back(keLibrary);

	//Create idle thread
	KThread* idle = KeCreateThread(&Kernel::IdleThread, this);
	idle->Name = "idle";

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
	Device* com1 = m_deviceTree.GetDeviceByName("COM1");
	Assert(com1 != nullptr);
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

	m_windows = new WindowingSystem(m_display);
	m_windows->Initialize();

	//Debugger
	m_debugger = new Debugger();
	m_debugger->Initialize();
	m_debugger->AddModule(keLibrary);

	//Done
	Print("Kernel Initialized\n");
}

void Kernel::HandleInterrupt(InterruptVector vector, PINTERRUPT_FRAME pFrame)
{
	if (m_debugger != nullptr && m_debugger->Enabled() && vector == InterruptVector::Breakpoint)
	{
		m_debugger->DebuggerEvent(vector, pFrame);
		return;
	}
	
	const auto& it = m_interruptHandlers->find(vector);
	if (it != m_interruptHandlers->end())
	{
		InterruptContext ctx = it->second;
		ctx.Handler(ctx.Context);
		HyperV::EOI();
		return;
	}

	this->Printf("ISR: 0x%x, Code: %x, RBP: 0x%16x, RIP: 0x%16x, RSP: 0x%16x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	this->Printf("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	this->Printf("  CS: 0x%x, SS: 0x%x\n", pFrame->CS, pFrame->SS);

	switch (vector)
	{
	case InterruptVector::PageFault:
		this->Printf("  CR2: 0x%16x\n", __readcr2());
		if (__readcr2() == 0)
			this->Printf("  Null pointer dereference\n", __readcr2());
	}

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
		uintptr_t base;
		if (context.Rip < KernelStart)
		{
			Assert(user);
			//User process
			base = user->GetProcess().GetModuleBase(context.Rip);
			Print("IP: 0x%016x Base: 0x%016x\n", context.Rip, base);
		}
		else
		{
			const KeLibrary* module = KeGetModule(context.Rip);
			Assert(module);
			base = (uintptr_t)module->Handle;
			const uint32_t rva = (uint32_t)(context.Rip - base);

			if (module->Pdb == nullptr)
			{
				kernel.Printf("IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", context.Rip, base, rva);
			}
			else
			{
				PdbFunctionLookup lookup = {};
				module->Pdb->ResolveFunction(rva, lookup);
				kernel.Printf("%s (%d)\n", lookup.Name, lookup.LineNumber);
			}
		}

		sw.Next(base);
	}

	ArchDisableInterrupts();
	while (true)
		ArchWait();
}

bool inBugcheck = false;
void Kernel::Bugcheck(const char* file, const char* line, const char* format, ...)
{
	cpu_flags_t flags = ArchDisableInterrupts();
	
	if (inBugcheck)
	{
		this->Printf("\n%s\n%s\n", file, line);
		va_list args;
		va_start(args, format);
		this->Printf(format, args);
		this->Printf("\n");
		va_end(args);

		while(true)
			ArchWait();
	}
	
	inBugcheck = true;
	
	if (m_scheduler != nullptr)
		m_scheduler->Enabled = false;

	if (m_timer != nullptr)
		m_timer->Disable();

	this->Printf("Kernel Bugcheck\n");
	this->Printf("\n%s\n%s\n", file, line);

	va_list args;
	va_start(args, format);
	this->Printf(format, args);
	this->Printf("\n");
	va_end(args);

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
	
	this->Printf("Call Stack\n");
	while (sw.HasNext())
	{
		const KeLibrary* module = KeGetModule(context.Rip);
		Assert(module);
		const uintptr_t base = (uintptr_t)module->Handle;
		const uint32_t rva = (uint32_t)(context.Rip - base);

		if (module->Pdb == nullptr)
		{
			kernel.Printf("IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", context.Rip, base, rva);
		}
		else
		{
			PdbFunctionLookup lookup = {};
			module->Pdb->ResolveFunction(rva, lookup);
			kernel.Printf("%s (%d)\n", lookup.Name.c_str(), lookup.LineNumber);
		}

		sw.Next(base);
	}

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
	//if ((m_debugger != nullptr) && m_debugger->IsBrokenIn())
		//m_debugger->KdpDprintf(format, args);
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
	if (m_scheduler && m_scheduler->Enabled)
		m_scheduler->Schedule();
}

KThread* Kernel::KeCreateThread(const ThreadStart start, void* arg, UserThread* userThread)
{
	//Current thread
	const KThread* current = m_scheduler->GetCurrentThread();
	Print("CreateThread - Id: 0x%x\n", current->GetId());
	
	//Add kernel thread
	KThread* thread = new KThread(start, arg, userThread);
	thread->InitContext(&Kernel::KernelThreadInitThunk);
	m_scheduler->AddReady(*thread);
	return thread;
}

void Kernel::KeExitThread()
{
	Print("Kernel::KeThreadExit\n");

	m_scheduler->KillThread();
}

Handle Kernel::KeLoadLibrary(const std::string& path)
{
	const Handle library = Loader::LoadKernelLibrary(path);

	const char* fullPath = PortableExecutable::GetPdbName(library);
	Assert(fullPath);
	const char* pdbName = GetFileName(fullPath);
	Assert(pdbName);

	const void* address = KeLoadPdb(pdbName);
	Pdb* pdb = new Pdb(address, (void*)library);
	this->Printf("KeLoadLibrary %s (0x%016x), %s (0x%016x)\n", path.c_str(), (uintptr_t)library, pdbName, address);

	KeLibrary keLibrary = { path, library, pdb };
	m_modules->push_back(keLibrary);

	if (m_debugger != nullptr)
		m_debugger->AddModule(keLibrary);

	return library;
}

const KeLibrary* Kernel::KeGetModule(const uintptr_t address) const
{
	for (const auto& module : *m_modules)
	{
		const uintptr_t imageBase = (uintptr_t)module.Handle;
		const size_t imageSize = PortableExecutable::GetSizeOfImage(module.Handle);

		if ((address >= imageBase) && (address < imageBase + imageSize))
			return &module;
	}
	return nullptr;
}

const KeLibrary* Kernel::KeGetModule(const std::string& path) const
{
	for (const auto& module : *m_modules)
	{
		if (module.Name == path)
			return &module;
	}
	return nullptr;
}

void Kernel::KernelThreadInitThunk()
{
	kernel.Printf("Kernel::KernelThreadInitThunk\n");
	KThread* current = kernel.m_scheduler->GetCurrentThread();
	current->Display();
	
	//Run thread
	current->Run();
	kernel.Printf("Kill thread: %d\n", current->GetId());

	//Exit thread TODO
	kernel.KeExitThread();
	Assert(false);
}

size_t Kernel::UserThreadInitThunk(void* unused)
{
	kernel.Printf("Kernel::UserThreadInitThunk\n");
	KThread* current = kernel.m_scheduler->GetCurrentThread();
	current->Display();

	UserThread* user = current->GetUserThread();
	user->DisplayDetails();

	user->GetProcess().Display();
	user->GetProcess().DisplayDetails();
	user->Run();

	Assert(false);
}

void Kernel::KeSleepThread(const nano_t value) const
{
	m_scheduler->Sleep(value);
}

void Kernel::KeGetSystemTime(SystemTime& time) const
{
	EFI_TIME efiTime = { };
	Assert(!EFI_ERROR(m_runtime.GetTime(&efiTime, nullptr)));

	time.Year = efiTime.Year;
	time.Month = efiTime.Month;
	//TODO: day of the week
	time.Day = efiTime.Day;
	time.Hour = efiTime.Hour;
	time.Minute = efiTime.Minute;
	time.Second = efiTime.Second;
	time.Milliseconds = efiTime.Nanosecond / 1000;
}

void* Kernel::Allocate(const size_t size, const uintptr_t callerAddress)
{
	return m_heap->Allocate(size, callerAddress);
}

void Kernel::Deallocate(void* address)
{
	m_heap->Deallocate(address);
}

void* Kernel::AllocateLibrary(const uintptr_t address, const size_t count)
{
	return m_virtualMemory->Allocate(address, count, MemoryProtection::PageReadWriteExecute, *m_librarySpace);
}

void* Kernel::AllocatePdb(const size_t count)
{
	return m_virtualMemory->Allocate(0, count, MemoryProtection::PageReadWrite, *m_pdbSpace);
}

void* Kernel::AllocateStack(const size_t count)
{
	return m_virtualMemory->Allocate(0, count, MemoryProtection::PageReadWrite, *m_stackSpace);
}

void* Kernel::AllocateWindows(const size_t count)
{
	return m_virtualMemory->Allocate(0, count, MemoryProtection::PageReadWrite, *m_windowsSpace);
}

void* Kernel::AllocateKernelPage(uintptr_t address, const size_t count, const MemoryProtection& protection)
{
	return m_virtualMemory->Allocate(address, count, protection, *m_heapSpace);
}

KSemaphore* Kernel::KeCreateSemaphore(const size_t initial, const size_t maximum, const std::string& name)
{
	KSemaphore* semaphore = new KSemaphore(initial, maximum, name);
	return semaphore;
}

bool Kernel::KeReleaseSemaphore(KSemaphore& semaphore, const size_t releaseCount)
{
	if (m_scheduler->Enabled)
	{
		cpu_flags_t flags = ArchDisableInterrupts();

		m_scheduler->SemaphoreRelease(&semaphore, releaseCount);

		ArchRestoreFlags(flags);
	}
	else
	{
		semaphore.Signal(releaseCount);
	}
	
	return true;
}

WaitStatus Kernel::KeWaitForSemaphore(KSemaphore& semaphore, size_t timeoutMs, size_t units)
{
	if (m_scheduler->Enabled)
	{
		cpu_flags_t flags = ArchDisableInterrupts();

		const nano100_t timeout = timeoutMs * 1000000 / 100;
		WaitStatus status = m_scheduler->SemaphoreWait(&semaphore, timeout);

		ArchRestoreFlags(flags);
		return status;
	}
	else
	{
		semaphore.Wait(units, timeoutMs);
		return WaitStatus::Signaled;
	}
}

bool Kernel::KeCloseSemaphore(KSemaphore* semaphore)
{
	semaphore->DecRefCount();

	if (semaphore->IsClosed())
		delete semaphore;

	return true;
}

void Kernel::KeRegisterInterrupt(const InterruptVector interrupt, const InterruptContext& context)
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
	return m_virtualMemory->VirtualMap((uintptr_t)address, addresses, protection, *m_runtimeSpace);
}

void* Kernel::VirtualMap(UserProcess& process, const void* address, const std::vector<paddr_t>& addresses, const MemoryProtection& protection)
{
	return m_virtualMemory->VirtualMap((uintptr_t)address, addresses, protection, process.GetAddressSpace());
}

Device* Kernel::KeGetDevice(const std::string& path) const
{
	return m_deviceTree.GetDevice(path);
}

void* Kernel::KeLoadPdb(const std::string& path)
{
	FileHandle* file = KeCreateFile(path, GenericAccess::Read);
	Assert(file);
	
	void* address = AllocatePdb(SIZE_TO_PAGES(file->Length));
	bool success = KeReadFile(*file, address, file->Length, nullptr);
	Assert(success);
	KeCloseFile(file);

	return address;
}

FileHandle* Kernel::KeCreateFile(const std::string& path, const GenericAccess access) const
{
	const Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	const RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver(); //TODO: figure out why HardDriveDriver didnt work

	FileHandle* handle = hdd->OpenFile(path, access);
	return handle;
}

bool Kernel::KeReadFile(FileHandle& file, void* buffer, const size_t bufferSize, size_t* bytesRead) const
{
	const Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	const RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver(); //TODO: figure out why HardDriveDriver didnt work

	const size_t read = hdd->ReadFile(file, buffer, bufferSize);
	file.Position += read;

	if (bytesRead != nullptr)
		*bytesRead = read;
	return true;
}

bool Kernel::KeSetFilePosition(FileHandle& file, const size_t position) const
{
	if (position >= file.Length)
		return false;

	file.Position = position;
	return true;
}

void Kernel::KeCloseFile(FileHandle* file) const
{
	delete file;
}

bool Kernel::KeCreateProcess(const std::string& path)
{
	Print("CreateProcess %s\n", path);
	FileHandle* file = KeCreateFile(path, GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(KeReadFile(*file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	Assert(KeSetFilePosition(*file, dosHeader.e_lfanew));
	Assert(KeReadFile(*file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	Assert(peHeader.Signature == IMAGE_NT_SIGNATURE);
	Assert(peHeader.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
	Assert(peHeader.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);
	
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
	Assert(KeSetFilePosition(*file, 0));
	Assert(KeReadFile(*file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	Assert(read == peHeader.OptionalHeader.SizeOfHeaders);

	PIMAGE_NT_HEADERS64 pNtHeader = MakePointer<PIMAGE_NT_HEADERS64>(address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)address + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			Assert(KeSetFilePosition(*file, section[i].PointerToRawData));
			Assert(KeReadFile(*file, (void*)destination, rawSize, &read));
			Assert(read == section[i].SizeOfRawData);
		}
	}
	KeCloseFile(file);

	//Load KernelAPI
	Handle api = Loader::LoadLibrary(*process, "mosrt.dll");
	Print("mosrt loaded at 0x%016x\n", api);

	//Save init pointers in library
	process->InitProcess = PortableExecutable::GetProcAddress(api, "InitProcess");
	process->InitThread = PortableExecutable::GetProcAddress(api, "InitThread");
	Print("Proc: 0x%016x Thread: 0x%016x\n", process->InitProcess, process->InitThread);

	//Patch imports of process for just mosrt
	Loader::KernelExports(address, api, "mosrt.dll");

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
	KThread* thread = KeCreateThread(&Kernel::UserThreadInitThunk, nullptr, userThread);
	
	char name[32] = {};
	sprintf(name, "%s[%d]", process.GetName().c_str(), userThread->GetId());
	thread->Name = name;
	process.AddThread(*thread);
	
	return thread;
}

void* Kernel::VirtualAlloc(UserProcess& process, const void* address, const size_t size, const MemoryAllocationType allocationType, const MemoryProtection protection)
{
	void* allocated = m_virtualMemory->Allocate((uintptr_t)address, SIZE_TO_PAGES(size), protection, process.GetAddressSpace());
	Assert(allocated);
	return allocated;
}

void Kernel::KePostMessage(Message* msg)
{
	if (m_windows != nullptr)
		m_windows->PostMessage(msg);
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

bool Kernel::IsValidKernelPointer(const void* p)
{
	//Make sure pointer is in Kernel's space
	if ((uintptr_t)p < KernelStart)
		return false;

	std::vector<VirtualAddressSpace*> addressSpaces = {
		m_librarySpace,
		m_pdbSpace,
		m_stackSpace,
		m_heapSpace,
		m_runtimeSpace,
		m_windowsSpace
	};

	//Check every address space
	for (auto& space : addressSpaces)
	{
		if (space->IsValidPointer(p))
			return true;
	}

	return false;
}

void Kernel::KePauseSystem() const
{
	ArchDisableInterrupts();
	m_scheduler->Enabled = false;
	//m_timer->Disable();
}

void Kernel::KeResumeSystem() const
{
	m_scheduler->Enabled = true;
	ArchEnableInterrupts();
	//m_timer->enable();
}
