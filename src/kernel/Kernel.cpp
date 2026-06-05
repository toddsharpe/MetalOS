#include "kernel/Kernel.h"

//Core crt
#include "core_crt/core_crt.c"

//Platform
#include "kernel/HyperV/Interrupts.cpp"
#include "kernel/HyperV/Timer.cpp"
#include "kernel/HyperV/Tsc.cpp"

//Architecture
#include "x64/CpuId.cpp"
#include "x64/PageTables.cpp"
#include "x64/PageTablesPool.cpp"

//Kernel
#include "kernel/VirtualMemoryManager.cpp"
#include "kernel/PhysicalMemoryManager.cpp"
#include "kernel/Scheduler.cpp"
#include "kernel/ConfigTables.cpp"
#include "kernel/KThread.cpp"
#include "kernel/KProcess.cpp"
#include "kernel/UProcess.cpp"
#include "kernel/LoadingScreen.cpp"
#include "kernel/Syscalls.cpp"
#include "kernel/UObject.cpp"
#include "kernel/WindowingSystem.cpp"
#include "kernel/Objects/KPipe.cpp"
#include "kernel/Debugger.cpp"

//Drivers
#include "kernel/Drivers/VmBusDriver.cpp"
#include "kernel/Drivers/HyperVChannel.cpp"
#include "kernel/Drivers/HyperVRingBuffer.cpp"
#include "kernel/Drivers/HyperVKeyboardDriver.cpp"
#include "kernel/Drivers/HyperVMouseDriver.cpp"
#include "kernel/Drivers/HyperVNic.cpp"

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

//Net
#include "kernel/Net/Arp.cpp"
#include "kernel/Net/ArpCache.cpp"
#include "kernel/Net/Checksum.cpp"
#include "kernel/Net/Ethernet.cpp"
#include "kernel/Net/Icmp.cpp"
#include "kernel/Net/IPv4.cpp"
#include "kernel/Net/Net.cpp"
#include "kernel/Net/Router.cpp"
#include "kernel/Net/Socket.cpp"
#include "kernel/Net/Udp.cpp"
#include "kernel/Net/Util.cpp"

/*
 * Bootloader parameters.
 */
LoaderParams BootParams;

/*
 * Kernel parameters.
 */
static constexpr size_t KernelArenaSize = PageSize << 10; //4MB Kernel Arena
static constexpr size_t AcpiArenaSize = PageSize << 8; //1MB ACPI Arena
static constexpr size_t TempArenaSize = PageSize << 10; //4MB Temp Arena
static constexpr size_t SharedArenaSize = PageSize << 10; //4MB Shared Arena
static constexpr size_t MaxUProcess = 32;
static constexpr CString RuntimeDLL = "mosrt.dll";

namespace
{
	/*
	 * Prototypes.
	 */
	void OnTimer0(void* const arg);
	uint32_t IdleThread(void* unused);

	/*
	 * Kernel Arenas.
	 */
	KERNEL_PAGE_ALIGN static StaticArena<KernelArenaSize> m_kernelArena;
	KERNEL_PAGE_ALIGN static StaticArena<AcpiArenaSize> m_acpiArena;
	KERNEL_PAGE_ALIGN static StaticArena<TempArenaSize> m_tempArena;
	KERNEL_PAGE_ALIGN static StaticArena<SharedArenaSize> m_sharedArena;

	ObjectPool<UProcess, MaxUProcess> m_procArena;

	/*
	* Kernel loader data.
	*/
	const LoaderParams& m_params(BootParams);
	EFI_RUNTIME_SERVICES m_runtime(*BootParams.Runtime);
	Graphics::FrameBuffer m_display((Graphics::Color*)KernelGraphicsDevice, BootParams.Display.VerticalResolution, BootParams.Display.HorizontalResolution);
	PageTablesPool m_pool((void*)KernelPageTablesPool, BootParams.PageTablesPoolAddress, BootParams.PageTablesPoolPageCount);
	ConfigTables m_configTables(BootParams.ConfigTables, BootParams.ConfigTablesCount);
	
	//Memory
	MemoryMap m_memoryMap(BootParams.MemoryMap.Table, BootParams.MemoryMap.Size, BootParams.MemoryMap.DescriptorSize);
	PhysicalMemoryManager m_physicalMemory((PageFrame*)KernelPageFrameDBStart, BootParams.PageFrameCount);

	//Basic output
	DirectUart m_uart(DirectUart::ComPort::Com1);
	LoadingScreen m_loadingScreen(m_display);

	//System
	InterruptTable<InterruptVector, IrqN> m_irqs;
	Scheduler m_scheduler(&HyperV::Tsc::ReadTsc);
	DeviceTree m_deviceTree;
	KProcess m_process(KProcessType::Kernel);
	WindowingSystem m_windows(m_display);
	Debugger m_debugger;

	Net::NetIf* m_netIf;

	void Initialize()
	{
		//Initialize architecture
		ArchInitialize();

		//Initialize Display
		m_loadingScreen.Initialize();

		//Initialize components that copy from UEFI space
		m_configTables.Initialize(m_kernelArena);
		m_memoryMap.Initialize(m_kernelArena);

		//Initialize paging
		m_pool.Initialize();
		PageTables::Pool = &m_pool;

		//Create kernel space and switch to it
		m_process.Initialize();

		/*
		 * Manually construct reservations/mappings for Kernel/PageTables pool.
		 * This avoids chicken/egg with VMM code executing and mapping from pool.
		 */
		PageTables pageTables;
		pageTables.CreateNew();
		pageTables.MapPages(KernelBase, m_params.KernelAddress, x64::SizeToPages(m_params.KernelImageSize), true);
		pageTables.MapPages(KernelPageTablesPool, m_params.PageTablesPoolAddress, m_params.PageTablesPoolPageCount, true);
		ArchSetPagingRoot(pageTables.GetRoot());

		m_process.Reserve(KernelBase, x64::SizeToPages(m_params.KernelImageSize));
		m_process.Reserve(KernelPageTablesPool, m_params.PageTablesPoolPageCount);

		/*
		 * Use VMM to map/reserve the rest.
		 */
		Assert(VMM::MapContiguous(m_process, (void*)KernelGraphicsDevice, m_params.Display.FrameBufferBase, x64::SizeToPages(m_params.Display.FrameBufferSize)));
		Assert(VMM::MapContiguous(m_process, (void*)KernelPageFrameDBStart, m_params.PageFrameAddr, x64::SizeToPages(BootParams.PageFrameCount * sizeof(PageFrame))));
		Assert(VMM::MapContiguous(m_process, (void*)KernelPdb, m_params.PdbAddress, x64::SizeToPages(m_params.PdbSize)));
		m_memoryMap.MapRuntime(m_process);
		
		//Continue initializing system
		m_physicalMemory.Initialize(m_memoryMap, m_kernelArena);
		m_irqs.Add(InterruptVector::Timer0, {&OnTimer0, nullptr});
		m_scheduler.Initialize();
		m_process.AddModule("moskrnl.exe", (void*)KernelBase);

		//Initialize Platform (HyperV)
		HyperV::Initialize();
		HyperV::Timer::Enable(Second / 128, 0, (uint8_t)InterruptVector::Timer0);

		//Welcome display
		m_loadingScreen.Printf("MetalOS.Kernel - Base:0x%016x Size: 0x%0X\n", m_params.KernelAddress, m_params.KernelImageSize);
		m_loadingScreen.Printf("  PhysicalAddressSize: 0x%016x\n", m_memoryMap.GetPhysicalAddressSize());

		//Test UEFI runtime access
		EFI_TIME time;
		m_runtime.GetTime(&time, nullptr);
		m_loadingScreen.Printf("  Date: %02d-%02d-%02d %02d:%02d:%02d\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

		//Initialize scheduler and create idle thread. Allows device enumeration to block for interrupts if needed
		KeCreateThread(&IdleThread, nullptr, "Idle");
		m_scheduler.Enabled = true;

		//Initialize ACPI/HW
		InitializeAcpi();
		m_deviceTree.Enumerate(m_kernelArena);
		if (BootParams.RamDriveAddress)
		{
			KDevice* ramDrive = m_kernelArena.Allocate<KDevice>();
			Assert(ramDrive);
			ramDrive->Hid = m_kernelArena.Copy(RamDriveHid);
			ramDrive->Type = KDeviceType::Software;
			ramDrive->Context = (void*)BootParams.RamDriveAddress;
			m_deviceTree.AddRootChild(*ramDrive, m_kernelArena);
		}

		m_process.Display();
		m_debugger.Initialize();
		m_debugger.AddModule(*m_process.GetModule("moskrnl.exe"));

		//Initialize windowing system
		m_windows.Initialize();

		//System displays
		m_kernelArena.Display();
		m_acpiArena.Display();
		m_scheduler.Display();
		m_deviceTree.Display();

		Printf("MetalOS Initialized!\n");
		PrintStack();

		//Start
		m_windows.Enabled = true;
	}

	void OnTimer0(void* arg)
	{
		HyperV::Interrupts::EOI();
		if (m_scheduler.Enabled)
			m_scheduler.Schedule();
	}

	uint32_t IdleThread(void* unused)
	{
		while (true)
			ArchWait();
	}
}

void KePauseSystem()
{
	ArchDisableInterrupts();
	m_scheduler.Enabled = false;
}

void KeResumeSystem()
{
	m_scheduler.Enabled = true;
	ArchEnableInterrupts();
}

bool KeIsValid(const void* address)
{
	return m_process.IsValidPointer(address);
}

/*
 * Globals with C++ linkage.
 */
paddr_t KePhysicalAlloc()
{
	paddr_t address = 0;
	Assert(m_physicalMemory.Allocate(address));

	return address;
}

paddr_t KePhysicalAlloc(const size_t count)
{
	paddr_t address = 0;
	Assert(m_physicalMemory.Allocate(address, count));
	
	return address;
}

void KePhysicalFree(const paddr_t address, const size_t count)
{
	Assert(false);
}

milli_t KeGetTicks()
{
	const nano100_t tsc = HyperV::Tsc::ReadTsc();
	return ToMilli(tsc);
}

nano_t KeGetNanoseconds()
{
	const nano100_t tsc = HyperV::Tsc::ReadTsc();
	return tsc * 100;
}

void KeGetSystemTime(KSystemTime& time)
{
	EFI_TIME efiTime = {};
	m_runtime.GetTime(&efiTime, nullptr);

	time.Year = efiTime.Year;
	time.Month = efiTime.Month;
	time.Day = efiTime.Day;
	time.Hour = efiTime.Hour;
	time.Minute = efiTime.Minute;
	time.Second = efiTime.Second;
	time.Milliseconds = efiTime.Nanosecond / 1000;
	time.Nanoseconds = efiTime.Nanosecond;
}

void* KeVirtualAlloc(const size_t size)
{
	return KeVirtualAlloc(m_process, nullptr, size);
}

void* KeVirtualAlloc(const void* address, const size_t size)
{
	return KeVirtualAlloc(m_process, address, size);
}

void* KeVirtualAlloc(KProcess& process, const void* address, const size_t size)
{
	const size_t count = x64::SizeToPages(size);
	if (!address)
	{
		return VMM::Allocate(process, count);
	}
	else
	{
		return VMM::Allocate(process, address, count);
	}
}

void* KeHeapAlloc(const size_t size, const HeapAllocType type)
{
	switch (type)
	{
		case HeapAllocType::Acpi:
			return m_acpiArena.Allocate(size);
			break;

		default:
			NotImplemented();
			return nullptr;
	}
}

void KeHeapFree(void* ptr, const HeapAllocType type)
{
	switch (type)
	{
		case HeapAllocType::Acpi:
			return m_acpiArena.Deallocate(ptr);
			break;

		default:
			NotImplemented();
	}
}

KThread* KeCreateThread(const KThreadStart start, void* const arg, const CString& name)
{
	KThread* ret = m_scheduler.CreateReady(start, arg, name);
	return ret;
}

void KeExitThread()
{
	KThread& current = m_scheduler.GetThread();
	KeExitThread(current);
}

void KeExitThread(KThread& thread)
{
	m_scheduler.KillThread(thread);
}

void KeSleepThread(const nano_t time)
{
	m_scheduler.Sleep(time);
}

void KeYield()
{
	m_scheduler.Schedule();
}

void KThreadInit()
{
	KThread& current = m_scheduler.GetCurrentThread();

	//Run thread
	current.Start();
	Printf("Thread exit: %d\n", current.Id);

	//Exit thread
	KeExitThread();

	Unreachable();
}

//If entry is null, use InitProcess. Otherwise use InitThread with this as its arg
UThread* KeCreateUThread(UProcess& process, const size_t stackSize, const UThreadStart entry, void* const arg)
{
	//Create kernel thread
	KThread* kThread = KeCreateThread(&UThreadInit, nullptr, "");
	Assert(kThread);
	kThread->UserThread = process.CreateThread(*kThread, stackSize, entry, arg);
	return kThread->UserThread;
}

uint32_t UThreadInit(void* const arg)
{
	UNUSED(arg);

	KThread& current = m_scheduler.GetCurrentThread();
	Assert(current.UserThread);
	UThread& user = *current.UserThread;
	user.Start();

	//TODO(tsharpe): Exit code
	return 0;
}

KModule* KeLoadLibrary(const CString& name)
{
	Printf("LoadLibrary: %s\n", name.c_str());

	//Check if module exists in process
	KModule* search = m_process.GetModule(name);
	if (search != nullptr)
		return search;

	//If it doesnt exists attempt to load it
	void* address = Loader::Load(m_process, name);
	if (!address)
		return nullptr;

	KModule* created = m_process.AddModule(name, address);
	return created;
}

//Since this method changes page tables, make sure every return restores original
UProcess* KeCreateProcess(const CString& cmd)
{
	//TODO(tsharpe): Make this not necessary
	PageTables current; 
	current.OpenCurrent();

	//Get path
	size_t i = 0;
	while (i < cmd.Length && cmd[i] != '\0' && cmd[i] !=' ')
		i++;
	Assert(i > 0);

	StaticString<64> path;
	path.Append(CString(cmd.c_str(), i));

	//Create UProcess
	UProcess* created = m_procArena.Allocate(path);
	Assert(created);
	created->Initialize();

	//Initialize and switch to new page tables
	created->Tables.CreateNew();
	created->Tables.LoadKernelMappings();
	ArchSetPagingRoot(created->Tables.GetRoot());

	//Load exe and runtime into process
	void* address = Loader::Load(*created, path);
	if (!address)
	{
		ArchSetPagingRoot(current.GetRoot());
		return nullptr;
	}
	Printf("Image: 0x%016X\n", address);

	void* runtime = Loader::Load(*created, RuntimeDLL);
	if (!runtime)
	{
		ArchSetPagingRoot(current.GetRoot());
		return nullptr;
	}
	Printf("runtime: 0x%016X\n", runtime);

	//Resolve runtime imports
	Loader::ResolveImports(address, runtime, RuntimeDLL);

	//Init pointers
	created->InitProcess = WinPE::GetProcAddress(runtime, "InitProcess");
	Assert(created->InitProcess);
	created->InitThread = WinPE::GetProcAddress(runtime, "InitThread");
	Assert(created->InitThread);
	
	//Update process structures
	created->InContextInit(address, cmd);

	created->AddModule(path, address);
	created->AddModule(RuntimeDLL, runtime);

	created->IsConsole = WinPE::IsConsole(address);

	//Create main process thread
	const size_t stackSize = WinPE::GetStackSize(address);
	KeCreateUThread(*created, stackSize);
	ArchSetPagingRoot(current.GetRoot());

	//Add debug output as stdout for now
	created->CreateObject(UObjectType::Debug, Handles::StdOut);
	created->CreateObject(UObjectType::Debug, Handles::StdErr);

	return created;
}

void KeTerminateProcess(UProcess& process, const uint32_t exitCode)
{
	Printf("Process: %s exited with code 0x%x\n", process.Name.c_str(), exitCode);
	
	//Close all objects
	UObject* top = process.GetObject();
	while (top != nullptr)
	{
		//TODO(tsharpe): Should this call internal method?
		CloseHandle((Handle)top->Handle);

		//Get next object
		top = process.GetObject();
	}

	//Kill all KThreads in process
	m_scheduler.KillProcess(process);
}

bool KeCreateFile(KFile& file, const CString& path, const KFileAccess access)
{
	const KDevice* device = m_deviceTree.GetDeviceByClass(KDeviceClass::Storage);
	Assert(device);
	StorageDriver* const storage = (StorageDriver*)device->Driver;
	Assert(storage);
	const Result result = storage->OpenFile(file, path, access);
	file.Driver = storage;
	
	return result == Result::Success ? true : false;
}

bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	const StorageDriver* storage = (StorageDriver*)file.Driver;
	Assert(storage);

	const size_t read = storage->ReadFile(file, buffer, bufferSize);
	file.Position += read;

	if (bytesRead != nullptr)
		*bytesRead = read;
	return true;
}

bool KeSetFilePosition(KFile& file, const size_t position)
{
	if (position >= file.Length)
		return false;

	file.Position = position;
	return true;
}

void* KeLoadFile(const CString& path)
{
	KFile file = {};
	Assert(KeCreateFile(file, path, KFileAccess::Read));

	void* const address = KeVirtualAlloc(nullptr, file.Length);
	Assert(KeReadFile(file, address, file.Length, nullptr));
	return address;
}

//Should be deprecated for VirtualMap
void* MapPages(const paddr_t address, const size_t pageCount, const MapType type)
{
	//TODO(tsharpe): this should be calling into VMM
	uintptr_t virtualBase = 0;
	switch (type)
	{
		case MapType::Acpi:
			virtualBase = KernelAcpiStart;
			break;

		case MapType::Driver:
			virtualBase = KernelIoStart;
			break;

		default:
			NotImplemented();
			return nullptr;
	}

	PageTables tables;
	tables.OpenCurrent();
	Assert(tables.MapPages(virtualBase + address, address, pageCount, true));
	return (void *)(virtualBase + address);
}

void* KeVirtualMap(const paddr_t* addresses, const size_t count)
{
	return KeVirtualMap(m_process, addresses, count);
}

void* KeVirtualMap(KProcess& process, const paddr_t* addresses, const size_t count)
{
	return VMM::VirtualMap(process, addresses, count);
}

KWaitResult KeWait(KSignalObject& obj, const milli_t timeout)
{
	return m_scheduler.ObjectWait(obj, timeout);
}

KPipe* KeCreatePipe(const size_t size)
{
	KPipe* created = m_sharedArena.Allocate<KPipe>(size);
	created->Init();
	return created;
}

void PrintStack()
{
	Context context = {};
	ArchSaveContext(&context);
	PrintStack(&context, m_process);
}

void PrintStack(const Context* context, const KProcess& process)
{
	//Convert to unwind context
	//NOTE(tsharpe): Convert unwind code to work on X64_CONTEXT
	CONTEXT ctx = {};
	ctx.Rip = context->Rip;
	ctx.Rsp = context->Rsp;
	ctx.Rbp = context->Rbp;

	Arena& arena = m_tempArena;
	arena.Reset();
	StaticMap<KModule*, PdbFile*, 32> map;

	Printf("Call Stack\n");
	StackWalk sw(&ctx);
	while (sw.HasNext())
	{
		//Get module
		KModule* module = process.GetModule(ctx.Rip);
		Printf("name: %s\n", module->Name.c_str());
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
				void* const loaded = KeLoadFile(CString(pdbName));

				//Load PdbFile
				pdb = arena.Allocate<PdbFile>();
				pdb->Open(loaded, module->ImageBase, arena);
				map.Add(module, pdb);
			}

			//Calculate RVA
			const uint32_t rva = (uint32_t)(ctx.Rip - (uintptr_t)module->ImageBase);

			//Lookup in PDB
			PdbLookup lookup = {};
			bool success = pdb->Resolve(rva, lookup);
			if (success)
			{
				Printf("    %s (%d)\n", lookup.Function.c_str(), lookup.Line);
			}
			else
			{
				Printf("    <resolve failed>\n");
			}
			Printf("        IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", ctx.Rip, module->ImageBase, rva);
		}
		else
		{
			//No module info for the IP, print it and exit
			Printf("    IP: 0x%016x\n", ctx.Rip);
			break;
		}
		sw.Next((uintptr_t)module->ImageBase);
	}
}

void KeRegisterInterrupt(const InterruptVector interrupt, const ActionContext& context)
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

void Printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	Printf(format, args);

	va_end(args);
}

void Printf(const char* format, va_list args)
{
	char buffer[1024];
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	m_uart.Write(buffer);
}

void CPrintf(const bool enabled, const char* format, ...)
{
	if (!enabled)
		return;

	va_list args;
	va_start(args, format);
	Printf(format, args);
	va_end(args);
}

void PrintBytes(const void* data, const size_t length)
{
	const uint32_t width = 16;
	const uint8_t* pData = reinterpret_cast<const uint8_t*>(data);
	
	//Print headers
	Printf("A: 0x%016x S: 0x%016x\n", data, length);
	Printf("---- ");
	for (size_t i = 0; i < width; i++)
	{
		Printf("%02x ", (unsigned char)i);
	}
	Printf("\n");

	char line[width] = { 0 };
	for (size_t i = 0; i < length; i++)
	{
		if (i != 0 && i % width == 0)
		{
			//Print characters
			Printf(" | ");
			for (const auto c : line)
			{
				Printf("%c", isprint(c) ? c : '.');
			}

			memset(line, 0, width);
			Printf("\n");
		}

		if (i % width == 0)
			Printf("%02x - ", (i / width) << 4);

		Printf("%02x ", (unsigned char)pData[i]);
		line[i % width] = (unsigned char)pData[i];
	}

	//Print the rest of the line
	if ((length % width) != 0)
		for (size_t i = 0; i < width - (length % width); i++)
		{
			Printf("   ");
		}

	//Print characters
	Printf(" | ");
	for (size_t i = 0; i < (length % width); i++)
	{
		char c = line[i];
		Printf("%c", isprint(c) ? c : '.');
	}

	Printf("\n");
}

void Bugcheck(const char* file, const char* line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	Bugcheck(file, line, format, args);
	va_end(args);
}

void Bugcheck(const char* file, const char* line, const char* format, va_list args)
{
	static bool inBugcheck = false;
	KePauseSystem();

	if (inBugcheck)
	{
		//Bugcheck during bugcheck, print what's available and bail
		Printf("Double bug check!\n");
		Printf("\n%s\n%s\n", file, line);
		Printf(format, args);
		Printf("\n");

		while (true)
			ArchWait();
	}
	inBugcheck = true;
	
	Printf("Bugcheck\r\n");
	Printf("    %s\n", file);
	Printf("    %s\n", line);
	
	Printf(format, args);
	Printf("\n");

	Context context = {};
	ArchSaveContext(&context);
	PrintStack(&context, m_process);

	while(true);
}

UWindow* CreateWindow(UThread& owner)
{
	return m_windows.CreateWindow(owner);
}

void Delete(UWindow& window)
{
	Assert(m_windows.Delete(&window));
}

//TODO(tsharpe): Should efi display be in the device tree?
Rectangle GetScreenRect2()
{
	return {0, 0, m_display.Width, m_display.Height};
}

KDevice* KeGetDevice(const CString& path)
{
	return m_deviceTree.GetDevice(path);
}

void KePostMessage(Message& msg)
{
	m_windows.PostMessage(msg);
}

/*
 * Globals with C linkage.
 */
extern "C"
{
	void KeInterrupt(const InterruptVector vector, InterruptFrame& frame)
	{
		if (vector == InterruptVector::DoubleFault)
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
		KThread& current = m_scheduler.GetCurrentThread();
		if (current.UserThread && current.UserThread->Process.IsValidPointer((void*)frame.RIP))
		{
			UProcess& proc = current.UserThread->Process;
			UObject* obj = proc.GetObject((handle_t)Handles::StdOut);
			if (obj && obj->Type == UObjectType::Pipe)
			{
				KPipe& pipe = obj->Pipe->KPipe;

				//Write exception
				switch (vector)
				{
					case InterruptVector::DivideError:
						pipe.Print("Exception: Divide by zero\n");
						break;

					case InterruptVector::Breakpoint:
						pipe.Print("Exception: Breakpoint\n");
						break;

					case InterruptVector::PageFault:
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
				StaticMap<KModule*, PdbFile*, 32> map;
				StackWalk sw(&ctx);
				while (sw.HasNext())
				{
					//Get module
					KModule* module = proc.GetModule(ctx.Rip);
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
							void* const loaded = KeLoadFile(CString(pdbName));

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
			case InterruptVector::PageFault:
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
			Context context = {};
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

		//TODO: start init process

		KeCreateProcess("init.exe from_kernel");
		KeExitThread();

		return 0;
	}
}
