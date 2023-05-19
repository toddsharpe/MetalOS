#include "Kernel.h"
#include "Assert.h"

#define PACKED
#include <tianocore-edk2/IndustryStandard/PlatformAcpi.h>
#include <intrin.h>
#include <cstdarg>
#include "System.h"
#include "Main.h"
#include <functional>
#include "Devices/AcpiDevice.h"
#include "Drivers/UartDriver.h"
#include "RuntimeSupport.h"
#include "PortableExecutable.h"
#include "HyperVTimer.h"
#include "HyperV.h"
#include "KernelHeap.h"
#include "BootHeap.h"
#include "StackWalk.h"
#include "Objects/KSemaphore.h"
#include "Drivers/RamDriveDriver.h"
#include "Devices/SoftwareDevice.h"
#include "Drivers/IoApicDriver.h"
#include "Loader.h"
#include <string>
#include "KThread.h"
#include <kernel/MetalOS.Arch.h>
#include <x64/PageTables.h>
#include <boot/PageTablesPool.h>
#include <boot/Path.h>
#include <kernel/MetalOS.Internal.h>
#include "EarlyUart.h"

#define ENABLE_PDB 1

Kernel::Kernel() :
	m_physicalAddress(),
	m_imageSize(),
	m_runtime(),
	m_pdbAddress(),
	m_pdbSize(),

	m_display(),
	m_loadingScreen(m_display),
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

	m_scheduler(),
	m_processes(),
	m_objectsRingBuffers(),

	m_modules(),

	m_hyperV(),

	m_deviceTree(),

	m_timer(0),

	m_pdb(),

	m_windows(m_display),

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
	m_display.Init((void*)KernelGraphicsDevice, params->Display);
	m_loadingScreen.Initialize();
	m_printer = &m_loadingScreen;

	//Initialize and process memory map
	m_memoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, *params->MemoryMap);
	m_memoryMap->ReclaimBootPages();
	m_memoryMap->MergeConventionalPages();
	//UEFI configuration tables (saves them to boot heap)
	m_configTables = new ConfigTables(params->ConfigTables, params->ConfigTableSizes);

	//Initialize page table pool
	PageTables::Pool = new PageTablesPool((void*)KernelPageTablesPool, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);

	//Construct kernel page table
	m_pageTables = new PageTables();

	m_pageTables->MapKernelPages(KernelBaseAddress, params->KernelAddress, SizeToPages(params->KernelImageSize));
	m_pageTables->MapKernelPages(KernelPageTablesPool, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	m_pageTables->MapKernelPages(KernelGraphicsDevice, params->Display.FrameBufferBase, SizeToPages(params->Display.FrameBufferSize));
	m_pageTables->MapKernelPages(KernelPfnDbStart, params->PfnDbAddress, SizeToPages(params->PfnDbSize));
	m_pageTables->MapKernelPages(KernelKernelPdb, params->PdbAddress, SizeToPages(params->PdbSize));

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

	this->Printf("MetalOS.Kernel - Base:0x%16x Size: 0x%x\n", m_physicalAddress, m_imageSize);
	this->Printf("  PhysicalAddressSize: 0x%16x\n", m_memoryMap->GetPhysicalAddressSize());

	//Test UEFI runtime access
	EFI_TIME time;
	m_runtime.GetTime(&time, nullptr);
	this->Printf("  Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);
	
	//Initialize virtual memory
	m_pfnDb = new PhysicalMemoryManager(*m_memoryMap);
	Assert(m_pfnDb->GetSize() == pfnDbSize);
	m_virtualMemory = new VirtualMemoryManager(*m_pfnDb);

	//TODO: Remove this concept, cant allocate memory or have growing containers before heap!
	m_heapSpace = new VirtualAddressSpace(KernelHeapStart, KernelHeapEnd, true);
	
	//Initialize Heap
	m_heap = new KernelHeap(*m_virtualMemory, *m_heapSpace);
	m_heapInitialized = true;
	m_printer = new EarlyUart(ComPort::Com1);

	//Initialize address spaces (TODO: condense with split method)
	m_librarySpace = new VirtualAddressSpace(KernelLibraryStart, KernelLibraryEnd, true);
	m_pdbSpace = new VirtualAddressSpace(KernelPdbStart, KernelPdbEnd, true);
	m_stackSpace = new VirtualAddressSpace(KernelStackStart, KernelStackEnd, true);
	m_heapSpace = new VirtualAddressSpace(KernelHeapStart, KernelHeapEnd, true);
	m_runtimeSpace = new VirtualAddressSpace(KernelRuntimeStart, KernelRuntimeEnd, true);
	m_windowsSpace = new VirtualAddressSpace(KernelWindowsStart, KernelWindowsEnd, true);

	//Interrupts
	m_interruptHandlers = new std::map<X64_INTERRUPT_VECTOR, InterruptContext>();
	m_interruptHandlers->insert({ X64_INTERRUPT_VECTOR::Timer0, { &Kernel::OnTimer0, this} });
	m_interruptHandlers->insert({ X64_INTERRUPT_VECTOR::Breakpoint, { [](void* arg) { ((Kernel*)arg)->Printf("Debug Breakpoint Exception\n"); }, this} });

	//Test interrupts
	//__debugbreak();

	//Process and thread containers
	m_scheduler = new Scheduler();
	m_scheduler->Init();
	KeCreateThread(&Kernel::IdleThread, this, "Idle");

	m_processes = new std::list<std::shared_ptr<UserProcess>>();
	m_objectsRingBuffers = new std::map<std::string, UserRingBuffer*>();

	//Modules
	m_modules = new std::list<KeLibrary>();
	KeLibrary keLibrary = { "moskrnl.exe", (Handle)KernelBaseAddress, m_pdb };
	m_modules->push_back(keLibrary);

	//Initialize Platform
	m_hyperV.Initialize();

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
	m_timer.Enable(Second / 512, (uint8_t)X64_INTERRUPT_VECTOR::Timer0);
	//m_timer.Enable(Second / 32, (uint8_t)X64_INTERRUPT_VECTOR::Timer0);

	//Attach drivers and enumerate tree
	m_deviceTree.EnumerateChildren();
	m_deviceTree.Display();

	//Install interrupts
	//Device* com2;
	//Assert(m_deviceTree.GetDeviceByName("COM2", &com2));
	//UartDriver* com2Driver = ((UartDriver*)com2->GetDriver());
	//m_interruptHandlers->insert({ X64_INTERRUPT_VECTOR::COM2, { &UartDriver::OnInterrupt, com2Driver} });

	//Device* ioapic;
	//Assert(m_deviceTree.GetDeviceByName("IOAPIC", &ioapic));
	//IoApicDriver* ioapicDriver = ((IoApicDriver*)ioapic->GetDriver());
	//ioapicDriver->MapInterrupt(X64_INTERRUPT_VECTOR::COM2, 3);//TODO: find a way to get 3 from ACPI
	//ioapicDriver->UnmaskInterrupt(3);

	m_windows.Initialize();
	
	//Lastly, load the pdb
#if ENABLE_PDB
	m_pdb = new Pdb((void*)KernelKernelPdb, (Handle*)KernelBaseAddress);
	KeGetModule("moskrnl.exe")->Pdb = m_pdb;
#endif

	//Debugger
	m_debugger.Initialize();
	m_debugger.AddModule(keLibrary);

	//Done
	this->Printf("Kernel Initialized\n");
}

void Kernel::HandleInterrupt(X64_INTERRUPT_VECTOR vector, X64_INTERRUPT_FRAME* frame)
{
	if (vector == X64_INTERRUPT_VECTOR::DoubleFault)
	{
		KePauseSystem();
		__halt();
	}

	if (m_debugger.Enabled() && vector == X64_INTERRUPT_VECTOR::Breakpoint)
	{
		m_debugger.DebuggerEvent(vector, frame);
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

	//Show interrupt context
	this->Printf("ISR: 0x%x, Code: %x\n", vector, frame->ErrorCode);
	this->Printf("    RIP: 0x%016x\n", frame->RIP);
	this->Printf("    RBP: 0x%016x\n", frame->RBP);
	this->Printf("    RSP: 0x%016x\n", frame->RSP);
	this->Printf("    RAX: 0x%016x\n", frame->RAX);
	this->Printf("    RBX: 0x%016x\n", frame->RBX);
	this->Printf("    RCX: 0x%016x\n", frame->RCX);
	this->Printf("    RDX: 0x%016x\n", frame->RDX);
	this->Printf("    CS: 0x%x, SS: 0x%x\n", frame->CS, frame->SS);

	switch (vector)
	{
	case X64_INTERRUPT_VECTOR::PageFault:
		this->Printf("    CR2: 0x%16x\n", __readcr2());
		if (__readcr2() == 0)
			this->Printf("        Null pointer\n");
	}

	//Show interrupt stack
	X64_CONTEXT context = {};
	context.Rip = frame->RIP;
	context.Rsp = frame->RSP;
	context.Rbp = frame->RBP;
	this->ShowStack(&context);

	//Bugcheck
	Fatal("Unhandled exception");
}

void Kernel::Bugcheck(const char* file, const char* line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	Bugcheck(file, line, format, args);
	va_end(args);
}

void Kernel::Bugcheck(const char* file, const char* line, const char* format, va_list args)
{
	static bool inBugcheck = false;
	this->KePauseSystem();

	if (inBugcheck)
	{
		//Bugcheck during bugcheck, print what's available and bail
		this->Printf("\n%s\n%s\n", file, line);
		this->Printf(format, args);
		this->Printf("\n");

		while (true)
			ArchWait();
	}
	inBugcheck = true;

	this->Printf("Kernel Bugcheck\n");
	this->Printf("\n%s\n%s\n", file, line);

	this->Printf(format, args);
	this->Printf("\n");

	/*
	if (m_debugger.Enabled())
	{
		__debugbreak();
		return;
	}
	*/
	X64_CONTEXT context = {};
	ArchSaveContext(&context);
	this->ShowStack(&context);

	if (m_scheduler)
	{
		KThread& thread = m_scheduler->GetCurrentThread();
		thread.Display();
	}

	//Pause
	while (true)
		ArchWait();
}

void Kernel::ShowStack(const X64_CONTEXT* context)
{
	//Convert to unwind context
	//NOTE(tsharpe): Convert unwind code to work on X64_CONTEXT
	CONTEXT ctx = { 0 };
	ctx.Rip = context->Rip;
	ctx.Rsp = context->Rsp;
	ctx.Rbp = context->Rbp;

	this->Printf("Call Stack\n");
	StackWalk sw(&ctx);
	while (sw.HasNext())
	{
		PdbFunctionLookup lookup = {};
		ResolveIP(ctx.Rip, lookup);

		kernel.Printf("    %s (%d)\n", lookup.Name.c_str(), lookup.LineNumber);
		kernel.Printf("        IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", ctx.Rip, lookup.Base, lookup.RVA);

		if (lookup.Base == nullptr)
			break;
		
		sw.Next((uintptr_t)lookup.Base);
	}
}

bool Kernel::ResolveIP(const uintptr_t ip, PdbFunctionLookup& lookup)
{
	const KeLibrary* module = KeGetModule(ip);
	if (!module)
		return false;

	if (!module->Pdb)
		return false;

	lookup.Base = module->ImageBase;
	lookup.RVA = (uint32_t)(ip - (uintptr_t)lookup.Base);
	return module->Pdb->ResolveFunction(lookup.RVA, lookup);
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
		this->Printf("Could not AcpiInitializeSubsystem: %d\n", Status);
		ArchWait();
	}

	Status = AcpiInitializeTables(nullptr, 16, FALSE);
	if (ACPI_FAILURE(Status))
	{
		this->Printf("Could not AcpiInitializeTables: %d\n", Status);
		ArchWait();
	}

	//TODO: notify handlers

	/* Install the default address space handlers. */
	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_MEMORY, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		this->Printf("Could not initialise SystemMemory handler, %s!", AcpiFormatException(Status));
		ArchWait();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_IO, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		this->Printf("Could not initialise SystemIO handler, %s!", AcpiFormatException(Status));
		ArchWait();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_PCI_CONFIG, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		this->Printf("Could not initialise PciConfig handler, %s!", AcpiFormatException(Status));
		ArchWait();
	}

	Status = AcpiLoadTables();
	if (ACPI_FAILURE(Status))
	{
		this->Printf("Could not AcpiLoadTables: %d\n", Status);
		ArchWait();
	}

	//Local handlers should be installed here

	Status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		this->Printf("Could not AcpiEnableSubsystem: %d\n", Status);
		ArchWait();
	}

	Status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		this->Printf("Could not AcpiInitializeObjects: %d\n", Status);
		ArchWait();
	}

	this->Printf("ACPI Finished\n");
}

void Kernel::OnTimer0()
{
	if (m_scheduler && m_scheduler->Enabled)
		m_scheduler->Schedule();
}

std::shared_ptr<KThread> Kernel::KeCreateThread(const ThreadStart start, void* const arg, const std::string& name)
{
	//Add kernel thread
	std::shared_ptr<KThread> thread = std::make_shared<KThread>(start, arg);
	thread->Init(&Kernel::KernelThreadInitThunk);
	thread->Name = name;
	Printf("    Name: %s\n", name.c_str());
	m_scheduler->AddReady(thread);

	return thread;
}

void Kernel::KeExitThread()
{
	this->Printf("Kernel::KeThreadExit\n");

	m_scheduler->KillCurrentThread();
}

std::shared_ptr<KThread> Kernel::CreateThread(UserProcess& process, size_t stackSize, ThreadStart startAddress, void* arg, void* entry)
{
	//Create kernel thread
	std::shared_ptr<KThread> thread = KeCreateThread(&Kernel::UserThreadInitThunk, nullptr);

	//Attach user thread
	thread->UserThread = new UserThread(startAddress, arg, entry, stackSize, process);;
	thread->Name = process.Name + "[" + std::to_string(thread->UserThread->Id) + "]";
	process.AddThread(*thread);

	return thread;
}

KeLibrary& Kernel::KeLoadLibrary(const std::string& path)
{
	void* library = Loader::LoadKernelLibrary(path);

	const char* fullPath = PortableExecutable::GetPdbName(library);
	Assert(fullPath);
	const char* pdbName = GetFileName(fullPath);
	Assert(pdbName);

	const void* address = KeLoadPdb(pdbName);
	Pdb* pdb = nullptr;
#if ENABLE_PDB
	pdb = new Pdb(address, (void*)library);
#endif
	this->Printf("KeLoadLibrary %s (0x%016x), %s (0x%016x)\n", path.c_str(), (uintptr_t)library, pdbName, address);

	KeLibrary keLibrary = { path, library, pdb };
	m_modules->push_back(keLibrary);

	m_debugger.AddModule(keLibrary);

	return m_modules->back();
}

const KeLibrary* Kernel::KeGetModule(const uintptr_t address) const
{
	for (const auto& module : *m_modules)
	{
		const uintptr_t imageBase = (uintptr_t)module.ImageBase;
		const size_t imageSize = PortableExecutable::GetSizeOfImage(module.ImageBase);

		if ((address >= imageBase) && (address < imageBase + imageSize))
			return &module;
	}
	return nullptr;
}

KeLibrary* Kernel::KeGetModule(const std::string& path) const
{
	for (auto& module : *m_modules)
	{
		if (module.Name == path)
			return &module;
	}
	return nullptr;
}

void Kernel::KernelThreadInitThunk()
{
	kernel.Printf("Kernel::KernelThreadInitThunk\n");

	KThread& current = kernel.m_scheduler->GetCurrentThread();
	current.Display();
	
	//Run thread
	current.Run();
	kernel.Printf("Thread exit: %d\n", current.Id);

	//Exit thread
	kernel.KeExitThread();
}

size_t Kernel::UserThreadInitThunk(void* unused)
{
	kernel.Printf("Kernel::UserThreadInitThunk\n");

	UserThread& user = kernel.m_scheduler->GetCurrentUserThread();
	user.Display();
	user.DisplayDetails();

	//Run thread
	user.Run();
	kernel.Printf("User thread exit: %d\n", user.Id);

	return 0;
}

void Kernel::KeSleepThread(const nano_t value) const
{
	m_scheduler->Sleep(value);
}

void Kernel::KeGetSystemTime(SystemTime& time) const
{
	EFI_TIME efiTime = { };
	//Assert(!EFI_ERROR(m_runtime.GetTime(&efiTime, nullptr)));
	m_runtime.GetTime(&efiTime, nullptr);

	time.Year = efiTime.Year;
	time.Month = efiTime.Month;
	//TODO: day of the week
	time.Day = efiTime.Day;
	time.Hour = efiTime.Hour;
	time.Minute = efiTime.Minute;
	time.Second = efiTime.Second;
	time.Milliseconds = efiTime.Nanosecond / 1000;
}

void* Kernel::Allocate(const size_t size, void* const caller)
{
	return m_heap->Allocate(size, caller);
}

void Kernel::Deallocate(void* const address, void* const caller)
{
	m_heap->Deallocate(address, caller);
}

void* Kernel::AllocateLibrary(const void* address, const size_t count)
{
	return m_virtualMemory->Allocate(address, count, *m_librarySpace);
}

void* Kernel::AllocatePdb(const size_t count)
{
	return m_virtualMemory->Allocate(0, count, *m_pdbSpace);
}

void* Kernel::AllocateStack(const size_t count)
{
	return m_virtualMemory->Allocate(0, count, *m_stackSpace);
}

void* Kernel::AllocateWindows(const size_t count)
{
	return m_virtualMemory->Allocate(0, count, *m_windowsSpace);
}

WaitStatus Kernel::KeWait(KSignalObject& obj, const milli_t timeout)
{
	WaitStatus status = m_scheduler->ObjectWait(obj, ToNano(timeout));
	return status;
}

void Kernel::KeRegisterInterrupt(const X64_INTERRUPT_VECTOR interrupt, const InterruptContext& context)
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

void* Kernel::VirtualMap(const void* address, const std::vector<paddr_t>& addresses)
{
	return m_virtualMemory->VirtualMap(address, addresses, *m_runtimeSpace);
}

void* Kernel::VirtualMap(UserProcess& process, const void* address, const std::vector<paddr_t>& addresses)
{
	return m_virtualMemory->VirtualMap(address, addresses, process.GetAddressSpace());
}

Device* Kernel::KeGetDevice(const std::string& path) const
{
	return m_deviceTree.GetDevice(path);
}

void* Kernel::KeLoadPdb(const std::string& path)
{
	KFile file;
	Assert(KeCreateFile(file, path, GenericAccess::Read));
	
	void* const address = AllocatePdb(SizeToPages(file.Length));
	Assert(KeReadFile(file, address, file.Length, nullptr));
	return address;
}

bool Kernel::KeCreateFile(KFile& file, const std::string& path, const GenericAccess access) const
{
	const Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	const RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver();
	return hdd->OpenFile(file, path, access) == Result::Success ? true : false;
}

bool Kernel::KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead) const
{
	const Device* device = m_deviceTree.GetDeviceByType(DeviceType::Harddrive);
	const RamDriveDriver* hdd = (RamDriveDriver*)device->GetDriver();

	const size_t read = hdd->ReadFile(file, buffer, bufferSize);
	file.Position += read;

	if (bytesRead != nullptr)
		*bytesRead = read;
	return true;
}

bool Kernel::KeSetFilePosition(KFile& file, const size_t position) const
{
	if (position >= file.Length)
		return false;

	file.Position = position;
	return true;
}
//TODO: fold into syscall function
UserProcess* Kernel::KeCreateProcess(const std::string& path)
{
	this->Printf("CreateProcess %s\n", path);

	KFile file;
	Assert(KeCreateFile(file, path, GenericAccess::Read));

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader = {};
	Assert(KeReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader = {};
	Assert(KeSetFilePosition(file, dosHeader.e_lfanew));
	Assert(KeReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	Assert(peHeader.Signature == IMAGE_NT_SIGNATURE);
	Assert(peHeader.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
	Assert(peHeader.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);
	
	//Create new process to create new address space
	const bool isConsole = peHeader.OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
	UserProcess* process = new UserProcess(path, isConsole);

	//Swap to new process address space to load file
	//TODO: page into kernel, load, then remove and page into process so we dont switch address space
	ArchSetPagingRoot(process->GetCR3());
	this->Printf("NewCr3: 0x%016x\n", process->GetCR3());

	this->Printf("Base: 0x%016x\n", peHeader.OptionalHeader.ImageBase);

	//Allocate pages
	void* address = VirtualAlloc(*process, (void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage);
	Assert(address);

	//Init in context of address space
	process->Init(address);
	process->AddModule(path.c_str(), address);

	//Read headers
	Assert(KeSetFilePosition(file, 0));
	Assert(KeReadFile(file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
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
			Assert(KeSetFilePosition(file, section[i].PointerToRawData));
			Assert(KeReadFile(file, (void*)destination, rawSize, &read));
			Assert(read == section[i].SizeOfRawData);
		}
	}

	//Load KernelAPI
	Handle api = Loader::LoadLibrary(*process, "mosrt.dll");
	this->Printf("mosrt loaded at 0x%016x\n", api);

	//Save init pointers in library
	process->InitProcess = PortableExecutable::GetProcAddress(api, "InitProcess");
	process->InitThread = PortableExecutable::GetProcAddress(api, "InitThread");
	this->Printf("Proc: 0x%016x Thread: 0x%016x\n", process->InitProcess, process->InitThread);

	//Patch imports of process for just mosrt
	Loader::KernelExports(address, api, "mosrt.dll");

	//TODO(tsharpe): These processes don't have STD handles

	//Create thread TODO: reserve vs commit stack size
	CreateThread(*process, pNtHeader->OptionalHeader.SizeOfStackReserve, nullptr, nullptr, process->InitProcess);

	return process;
}

void* Kernel::VirtualAlloc(UserProcess& process, const void* address, const size_t size)
{
	kernel.Printf("VirtualAlloc: 0x%016x, Size: 0x%x\n", address, size);
	void* allocated = m_virtualMemory->Allocate(address, SizeToPages(size), process.GetAddressSpace());
	Assert(allocated);
	return allocated;
}

void Kernel::KePostMessage(Message& msg)
{
	m_windows.PostMessage(msg);
}

bool Kernel::IsValidUserPointer(const void* p)
{
	if (!p)
		return false;
	
	//Make sure pointer is in User's address half
	if ((uintptr_t)p > UserStop)
		return false;

	//Make sure pointer is User's address space
	UserProcess& process = m_scheduler->GetCurrentProcess();
	return process.GetAddressSpace().IsValidPointer(p);
}

bool Kernel::IsValidKernelPointer(const void* p)
{
	if (!p)
		return false;
	
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

void Kernel::KePauseSystem()
{
	ArchDisableInterrupts();
	m_timer.Disable();

	if (m_scheduler)
		m_scheduler->Enabled = false;
}

void Kernel::KeResumeSystem()
{
	if (m_scheduler)
		m_scheduler->Enabled = true;
	ArchEnableInterrupts();
	//TODO(tsharpe): Enable timer
	//m_timer.Enable();
}
