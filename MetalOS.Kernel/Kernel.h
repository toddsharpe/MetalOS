#pragma once

#include <efi.h>
#include <LoaderParams.h>
#include <MetalOS.h>
#include <WindowsPE.h>
#include "msvc.h"
#include "MetalOS.Kernel.h"
#include "Display.h"
#include "LoadingScreen.h"
#include <PageTablesPool.h>
#include "MemoryMap.h"
#include "ConfigTables.h"
extern "C"
{
#include <acpi.h>
}
#include <map>
#include "AcpiDeviceTree.h"
#include "UartDriver.h"

class Kernel
{
public:
	Kernel();
	::NO_COPY_OR_ASSIGN(Kernel);
	void Initialize(const PLOADER_PARAMS params);

	void HandleInterrupt(size_t vector, PINTERRUPT_FRAME pFrame);
	void Bugcheck(const char* file, const char* line, const char* assert);

	void Print(const char* format, ...);
	void Print(const char* format, va_list args);
	void PrintArray(char* buffer, size_t length);

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

private:
	void InitializeAcpi();
	PIMAGE_SECTION_HEADER GetKernelSection(const std::string& name);

private:
	//Save from LoaderParams
	uintptr_t m_physicalAddress;
	size_t m_imageSize;
	EFI_RUNTIME_SERVICES m_runtime;

	PageTablesPool* m_pPagePool;
	MemoryMap* m_pMemoryMap;
	ConfigTables* m_pConfigTables;
	PageTables* m_pageTables;

	Display* m_pDisplay;
	LoadingScreen* m_pLoading;

	size_t m_lastProcessId;
	std::list<KERNEL_PROCESS>* m_processes;

	Handle m_objectId;

	//Sections
	PIMAGE_SECTION_HEADER m_pdata;

	//Queues
	//std::queue<uint32_t> readyQueue;
	//std::list<uint32_t> sleepQueue;

	AcpiDeviceTree m_deviceTree;

	//Debug device
	AcpiDevice* m_com1;
};

