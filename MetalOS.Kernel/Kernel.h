#pragma once

#include "msvc.h"
#include <efi.h>
#include <LoaderParams.h>
#include <MetalOS.h>
#include <WindowsPE.h>
#include "MetalOS.Kernel.h"
#include "Display.h"
#include "TextScreen.h"
#include <PageTablesPool.h>
#include "MemoryMap.h"
#include "ConfigTables.h"
extern "C"
{
#include <acpi.h>
}
#include <map>
#include "DeviceTree.h"
#include "UartDriver.h"
#include "HyperVTimer.h"
#include "HyperV.h"
#include "PhysicalMemoryManager.h"
#include "VirtualAddressSpace.h"
#include "VirtualMemoryManager.h"
#include "Scheduler.h"
#include "KernelHeap.h"
#include <queue>
#include "Pdb.h"
#include "KThread.h"
#include "UserProcess.h"

class Kernel
{

public:
	Kernel();
	::NO_COPY_OR_ASSIGN(Kernel);
	void Initialize(const PLOADER_PARAMS params);

	void HandleInterrupt(InterruptVector vector, PINTERRUPT_FRAME pFrame);
	void __declspec(noreturn) Bugcheck(const char* file, const char* line, const char* assert);

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

	static uint32_t IdleThread(void* arg)
	{
		while (true)
			__halt();
	}

#pragma region Heap Interface
	void* Allocate(const size_t size, const uintptr_t callerAddress);
	void Deallocate(void* address);
#pragma endregion

#pragma region Virtual Memory Interface
	paddr_t AllocatePhysical(const size_t count);
	void* AllocateKernelPage(const uintptr_t address, const size_t count, const MemoryProtection& protect);
	void* VirtualMap(const void* address, const std::vector<paddr_t>& addresses, const MemoryProtection& protect);

	//User process address space
	void* VirtualAlloc(UserProcess& process, void* address, size_t size, MemoryAllocationType allocationType, MemoryProtection protect);
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
	void* DriverMapIoSpace(paddr_t PhysicalAddress, size_t NumberOfBytes);
	void* DriverMapPages(paddr_t address, size_t count);
#pragma endregion

#pragma region Kernel Interface
	Handle CreateThread(UserProcess& process, size_t stackSize, ThreadStart startAddress, void* arg, void* entry);
	static void KernelThreadInitThunk();
	static void UserThreadInitThunk();
	void KernelThreadSleep(nano_t value);
	void GetSystemTime(SystemTime* time);

	void RegisterInterrupt(const InterruptVector interrupt, const InterruptContext& context);

	Device* GetDevice(const std::string path);

	FileHandle* CreateFile(const std::string& path, GenericAccess access);
	bool ReadFile(FileHandle* file, void* buffer, size_t bufferSize, size_t* bytesRead);
	bool SetFilePosition(FileHandle* file, size_t position);
	void CloseFile(FileHandle* file);
	bool CreateProcess(const std::string& path);
#pragma endregion

#pragma region Semaphore Interface
	Handle CreateSemaphore(const size_t initial, const size_t maximum, const std::string& name);
	bool ReleaseSemaphore(Handle handle, const size_t releaseCount);
	WaitStatus WaitForSemaphore(Handle handle, size_t timeoutMs, size_t units = 1);
	bool CloseSemaphore(Handle handle);
#pragma endregion

	void CreateKernelThread(ThreadStart start, void* arg);
	uint64_t Syscall(SystemcallFrame* frame);

#pragma region System Calls
	uint64_t GetSystemInfo(struct SystemInfo* info);

	void Sleep(uint32_t milliseconds);
	uint64_t ExitProcess(uint32_t exitCode);

	uint32_t CreateWindow(const char* name);
	uint32_t GetWindowRect(Handle handle, Rectangle* rect);
	uint32_t GetMessage(struct Message* message);
	uint32_t PeekMessage(struct Message* message);

	uint64_t DebugPrint(char* s);

	uint64_t SetScreenBuffer(void* buffer);

	Handle CreateFile(const char* name, GenericAccess access);
	uint32_t ReadFile(Handle* handle, void* buffer, size_t bufferSize, size_t* bytesRead);
	uint32_t SetFilePointer(Handle* handle, __int64 position, enum FilePointerMove moveType);
	uint32_t CloseFile(Handle* handle);

	void* VirtualAlloc(void* address, size_t size, enum MemoryAllocationType allocationType, enum MemoryProtection protect);
#pragma endregion

	void PostMessage(Message* msg);

	UserWindow* Window;

private:
	void InitializeAcpi();

	static void OnTimer0(void* arg) { ((Kernel*)arg)->OnTimer0(); };
	void OnTimer0();

	KSemaphore* GetSemaphore(Handle handle)
	{
		return (KSemaphore*)handle;
	}

private:
	//Save from LoaderParams
	paddr_t m_physicalAddress;
	size_t m_imageSize;
	EFI_RUNTIME_SERVICES m_runtime;
	paddr_t m_pdbAddress;
	size_t m_pdbSize;

	//Basic output drivers
	Display* m_display;
	TextScreen* m_textScreen;
	StringPrinter* m_printer;

	//Initialize before PT switch
	MemoryMap* m_memoryMap;
	ConfigTables* m_configTables;
	PageTablesPool* m_pagePool;
	PageTables* m_pageTables;

	//Memory Management
	bool m_heapInitialized;
	paddr_t m_pfnDbAddress;
	PhysicalMemoryManager* m_pfnDb;
	VirtualMemoryManager* m_virtualMemory;
	VirtualAddressSpace* m_addressSpace;
	KernelHeap* m_heap;

	//Interrupts
	std::map<InterruptVector, InterruptContext>* m_interruptHandlers;

	//Process and Thread management
	std::map<uint32_t, UserProcess*>* m_processes;
	Scheduler* m_scheduler;
	
	//Platform
	HyperV* m_hyperV;

	//IO
	DeviceTree m_deviceTree;

	//TODO: should be an interface
	HyperVTimer* m_timer;

	//PDB
	Pdb* m_pdb;
};

