#pragma once

#include <efi.h>
#include "EfiDisplay.h"
#include "MemoryMap.h"
#include "ConfigTables.h"
extern "C"
{
#include <acpi.h>
}
#include <vector>
#include <list>
#include <map>
#include "Debugger.h"
#include "DeviceTree.h"
#include "HyperVTimer.h"
#include "HyperV.h"
#include "PhysicalMemoryManager.h"
#include "VirtualAddressSpace.h"
#include "VirtualMemoryManager.h"
#include "Scheduler.h"
#include "KernelHeap.h"
#include <queue>
#include "Pdb/Pdb.h"
#include "KThread.h"
#include "UserProcess.h"
#include "WindowingSystem.h"
#include "Objects/KEvent.h"
#include <kernel/MetalOS.Arch.h>
#include <shared/MetalOS.Types.h>
#include <user/MetalOS.h>
#include <LoadingScreen.h>

#include <memory>


class Kernel
{
	friend Debugger;
public:
	Kernel();
	Kernel(const Kernel&) = delete;
	Kernel& operator = (const Kernel&) = delete;

	void Initialize(const PLOADER_PARAMS params);

	void HandleInterrupt(X64_INTERRUPT_VECTOR vector, X64_INTERRUPT_FRAME* frame);
	__declspec(noreturn) void Bugcheck(const char* file, const char* line, const char* format, ...);
	__declspec(noreturn) void Bugcheck(const char* file, const char* line, const char* format, va_list args);
	void ShowStack(const X64_CONTEXT* context);
	bool ResolveIP(const uintptr_t ip, PdbFunctionLookup& lookup);

	void Printf(const char* format, ...);
	void Printf(const char* format, va_list args);
	void PrintBytes(const char* buffer, const size_t length)
	{
		this->m_printer->PrintBytes(buffer, length);
	}

	bool IsHeapInitialized()
	{
		return m_heapInitialized;
	}

	//This method only works because the loader ensures we are physically contiguous
	paddr_t VirtualToPhysical(uintptr_t virtualAddress)
	{
		//TODO: assert
		uint64_t rva = virtualAddress - KernelBaseAddress;
		return m_physicalAddress + rva;
	}

#pragma region Heap Interface
	void* Allocate(const size_t size, void* const caller);
	void Deallocate(void* const address, void* const caller);
#pragma endregion

#pragma region Virtual Memory Interface
	paddr_t AllocatePhysical(const size_t count);
	void* AllocateLibrary(const void* address, const size_t count);
	void* AllocatePdb(const size_t count);
	void* AllocateStack(const size_t count);
	void* AllocateWindows(const size_t count);
	void* VirtualMap(const void* address, const std::vector<paddr_t>& addresses);

	//User process address space
	void* VirtualAlloc(UserProcess& process, const void* address, const size_t size);
	void* VirtualMap(UserProcess& process, const void* address, const std::vector<paddr_t>& addresses);
#pragma endregion

#pragma region ACPI
	ACPI_STATUS AcpiOsInitialize();
	ACPI_STATUS AcpiOsTerminate();
	ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer();
	ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* PredefinedObject, ACPI_STRING* NewValue);
	ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable);
	void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length);
	void AcpiOsUnmapMemory(void* where, ACPI_SIZE length);
	ACPI_STATUS AcpiOsGetPhysicalAddress(void* LogicalAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress);
	void* AcpiOsAllocate(ACPI_SIZE Size);
	void AcpiOsFree(void* Memory);
	BOOLEAN AcpiOsReadable(void* Memory, ACPI_SIZE Length);
	BOOLEAN AcpiOsWritable(void* Memory, ACPI_SIZE Length);
	ACPI_THREAD_ID AcpiOsGetThreadId();
	ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context);
	void AcpiOsSleep(UINT64 Milliseconds);
	void AcpiOsStall(UINT32 Microseconds);
	ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle);
	ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle);
	void AcpiOsVprintf(const char* Format, va_list Args);
	ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout);
	ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units);
	ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle);
	void AcpiOsDeleteLock(ACPI_SPINLOCK Handle);
	ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle);
	void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags);
	ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info);
	ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width);
	ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width);
	ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width);
	ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width);
	UINT64 AcpiOsGetTimer();
	void AcpiOsWaitEventsComplete();
	ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64* Value, UINT32 Width);
	ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64 Value, UINT32 Width);
	ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void* Context);
	ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler);
	ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_PHYSICAL_ADDRESS* NewAddress, UINT32* NewTableLength);
#pragma endregion

#pragma region Driver Interface
	void* DriverMapPages(paddr_t address, size_t count);
#pragma endregion

#pragma region ThreadStarts
	__declspec(noreturn) static void KernelThreadInitThunk();
	static size_t UserThreadInitThunk(void* unused);
#pragma endregion

#pragma region Kernel Debugger
	void KePauseSystem();
	void KeResumeSystem();
#pragma endregion

#pragma region Internal Interface
	//Processes
	UserProcess* KeCreateProcess(const std::string& path);
	
	//Threads
	std::shared_ptr<KThread> KeCreateThread(const ThreadStart start, void* const arg, const std::string& name = "");
	void KeSleepThread(const nano_t value) const;
	void KeExitThread();
	std::shared_ptr<KThread> CreateThread(UserProcess& process, size_t stackSize, ThreadStart startAddress, void* arg, void* entry);

	//Libraries
	KeLibrary& KeLoadLibrary(const std::string& path);
	void* KeLoadPdb(const std::string& path);
	KeLibrary* KeGetModule(const std::string& path) const;
	const KeLibrary* KeGetModule(const uintptr_t address) const;

	//Files
	bool KeCreateFile(KFile& file, const std::string& path, const GenericAccess access) const;
	bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead) const;
	bool KeSetFilePosition(KFile& file, const size_t position) const;

	//Wait/Signals
	WaitStatus KeWait(KSignalObject& obj, const milli_t timeout = std::numeric_limits<milli_t>::max());

	void KeGetSystemTime(SystemTime& time) const;

	void KeRegisterInterrupt(const X64_INTERRUPT_VECTOR interrupt, const InterruptContext& context);

	Device* KeGetDevice(const std::string& path) const;

	uint64_t Syscall(X64_SYSCALL_FRAME* frame);
	bool IsValidUserPointer(const void* p);
	bool IsValidKernelPointer(const void* p);
	void KePostMessage(Message& msg);

#pragma endregion

#pragma region System Calls
	SystemCallResult GetSystemInfo(SystemInfo* info);
	size_t GetTickCount();
	SystemCallResult GetSystemTime(SystemTime* time);

	HThread GetCurrentThread();
	SystemCallResult CreateProcess(const char* processName, const CreateProcessArgs* args, CreateProcessResult* result);
	HThread CreateThread(size_t stackSize, ThreadStart startAddress, void* arg);
	uint32_t GetThreadId(const Handle handle);
	void Sleep(const uint32_t milliseconds);
	void SwitchToThread();
	SystemCallResult ExitProcess(const uint32_t exitCode);
	SystemCallResult ExitThread(const uint32_t exitCode);

	SystemCallResult AllocWindow(HWindow* handle, const Graphics::Rectangle* bounds);
	SystemCallResult PaintWindow(HWindow handle, const ReadOnlyBuffer* buffer);
	SystemCallResult MoveWindow(HWindow handle, const Graphics::Rectangle* bounds);
	SystemCallResult GetWindowRect(HWindow handle, Graphics::Rectangle* bounds);
	SystemCallResult GetMessage(Message* message);
	SystemCallResult PeekMessage(Message* message);
	SystemCallResult GetScreenRect(Graphics::Rectangle* rect);

	HFile CreateFile(const char* name, const GenericAccess access);
	SystemCallResult CreatePipe(HFile* readHandle, HFile* writeHandle);
	SystemCallResult ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead);
	SystemCallResult WriteFile(const HFile handle, const void* lpBuffer, const size_t bufferSize, size_t* bytesWritten);
	SystemCallResult SetFilePointer(const HFile handle, const size_t position, const FilePointerMove moveType, size_t* newPosition);
	SystemCallResult CloseFile(const HFile handle);
	SystemCallResult MoveFile(const char* existingFileName, const char* newFileName);
	SystemCallResult DeleteFile(const char* fileName);
	SystemCallResult CreateDirectory(const char* path);
	SystemCallResult WaitForSingleObject(const Handle handle, const uint32_t milliseconds, WaitStatus* status);
	SystemCallResult GetPipeInfo(const HFile handle, PipeInfo* info);
	SystemCallResult CloseHandle(const Handle handle);
	SystemCallResult CreateEvent(HEvent* handle, const bool manual, const bool initial);
	SystemCallResult SetEvent(const HEvent handle);
	SystemCallResult ResetEvent(const HEvent handle);

	void* VirtualAlloc(const void* address, const size_t size);
	HRingBuffer CreateRingBuffer(const char* name, const size_t indexSize, const size_t ringSize);
	HSharedMemory CreateSharedMemory(const char* name, const size_t size);
	void* MapObject(const void* address, Handle handle);
	void* MapSharedObject(const void* address, const char* name);

	SystemCallResult DebugPrint(const char* s);
	SystemCallResult DebugPrintBytes(const char* buffer, const size_t length);
#pragma endregion

private:
	static size_t IdleThread(void* unused)
	{
		while (true)
			ArchWait();
	}

	void InitializeAcpi();

	static void OnTimer0(void* arg) { ((Kernel*)arg)->OnTimer0(); };
	void OnTimer0();

private:
	//Save from LoaderParams
	paddr_t m_physicalAddress;
	size_t m_imageSize;
	EFI_RUNTIME_SERVICES m_runtime;
	paddr_t m_pdbAddress;
	size_t m_pdbSize;

	//Basic output drivers
	EfiDisplay m_display;
	LoadingScreen m_loadingScreen;
	StringPrinter* m_printer;

	//Initialize before PT switch
	MemoryMap* m_memoryMap;
	ConfigTables* m_configTables;
	PageTablesPool* m_pagePool;
	PageTables* m_pageTables;

	//Heap spaces
	VirtualAddressSpace* m_librarySpace;
	VirtualAddressSpace* m_pdbSpace;
	VirtualAddressSpace* m_stackSpace;
	VirtualAddressSpace* m_heapSpace;
	VirtualAddressSpace* m_runtimeSpace;
	VirtualAddressSpace* m_windowsSpace;

	//Memory Management
	bool m_heapInitialized;
	paddr_t m_pfnDbAddress;
	PhysicalMemoryManager* m_pfnDb;
	VirtualMemoryManager* m_virtualMemory;
	KernelHeap* m_heap;

	//Interrupts
	std::map<X64_INTERRUPT_VECTOR, InterruptContext>* m_interruptHandlers;

	//Process and Thread management
	Scheduler* m_scheduler;
	std::map<std::string, UserRingBuffer*> *m_objectsRingBuffers;
	std::list<std::shared_ptr<UserProcess>>* m_processes;

	//Libraries
	std::list<KeLibrary>* m_modules;

	//Platform
	HyperV m_hyperV;

	//IO
	DeviceTree m_deviceTree;
	HyperVTimer m_timer;

	//PDB
	Pdb* m_pdb;

	//UI
	WindowingSystem m_windows;

	Debugger m_debugger;
};
extern Kernel kernel;

