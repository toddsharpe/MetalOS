#include "Kernel.h"
#include <intrin.h>
#include "x64.h"
#include "KSpinLock.h"
#include "KSemaphore.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define Trace() Print(__FILE__ "-" STR(__LINE__));

ACPI_STATUS Kernel::AcpiOsInitialize()
{
	return ACPI_STATUS();
}

ACPI_STATUS Kernel::AcpiOsTerminate()
{
	return ACPI_STATUS();
}

ACPI_PHYSICAL_ADDRESS acpiRoot = 0;
ACPI_PHYSICAL_ADDRESS Kernel::AcpiOsGetRootPointer()
{
	if (acpiRoot == 0)
		acpiRoot = (ACPI_PHYSICAL_ADDRESS)m_configTables->GetAcpiTable();
	return acpiRoot;
}

ACPI_STATUS Kernel::AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* PredefinedObject, ACPI_STRING* NewValue)
{
	*NewValue = 0;
	return AE_OK;
}

ACPI_STATUS Kernel::AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable)
{
	*NewTable = nullptr;
	return AE_OK;
}

void* Kernel::AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{
	//Handle unaligned addresses
	size_t pageOffset = PhysicalAddress & PAGE_MASK;
	size_t pageCount = SIZE_TO_PAGES(pageOffset + Length);

	uintptr_t physicalBase = PhysicalAddress & ~PAGE_MASK;
	m_pageTables->MapKernelPages(KernelACPIAddress + physicalBase, physicalBase, pageCount);

	return (void*)(KernelACPIAddress + physicalBase + pageOffset);
}

void Kernel::AcpiOsUnmapMemory(void* where, ACPI_SIZE length)
{
	//Trace();
	//TODO
}

ACPI_STATUS Kernel::AcpiOsGetPhysicalAddress(void* LogicalAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress)
{
	PageTables* current = new PageTables(__readcr3());
	*PhysicalAddress = current->ResolveAddress((uintptr_t)LogicalAddress);

	return AE_OK;
}

void* Kernel::AcpiOsAllocate(ACPI_SIZE Size)
{
	return operator new((size_t)Size);
}

void Kernel::AcpiOsFree(void* Memory)
{
	operator delete(Memory);
}

BOOLEAN Kernel::AcpiOsReadable(void* Memory, ACPI_SIZE Length)
{
	return TRUE;
}
BOOLEAN Kernel::AcpiOsWritable(void* Memory, ACPI_SIZE Length)
{
	return TRUE;
}

ACPI_THREAD_ID Kernel::AcpiOsGetThreadId()
{
	KThread* thread = m_scheduler->GetCurrentThread();
	return thread->GetId();
}

ACPI_STATUS Kernel::AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context)
{
	//TODO
	Assert(false);
	return AE_ERROR;
}

void Kernel::AcpiOsSleep(UINT64 Milliseconds)
{
	Assert(false);
}

void Kernel::AcpiOsStall(UINT32 Microseconds)
{
	Assert(false);
}

ACPI_STATUS Kernel::AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle)
{
	*OutHandle = this->CreateSemaphore(InitialUnits, MaxUnits, "AcpiSemaphore");
	return AE_OK;
}

ACPI_STATUS Kernel::AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
	if (!Handle)
		return AE_BAD_PARAMETER;

	this->CloseSemaphore(Handle);
	return AE_OK;
}

void Kernel::AcpiOsVprintf(const char* Format, va_list Args)
{
	//Reduce ACPI talk for now (soon, pump to uart?)
	//Print(Format, Args);
}

ACPI_STATUS Kernel::AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
	if (!Handle)
		return AE_BAD_PARAMETER;

	Assert(Units == 1);

	WaitStatus status = this->WaitForSemaphore(Handle, Timeout, Units);
	if (status == WaitStatus::Signaled)
		return AE_OK;
	else if (status == WaitStatus::Timeout)
		return AE_TIME;
}

ACPI_STATUS Kernel::AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
	if (!Handle)
		return AE_BAD_PARAMETER;
	
	if (!this->ReleaseSemaphore(Handle, Units))
		return AE_BAD_PARAMETER;
	
	return AE_OK;
}

ACPI_STATUS Kernel::AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle)
{
	*OutHandle = new KSpinLock();
	return AE_OK;
}

void Kernel::AcpiOsDeleteLock(ACPI_SPINLOCK Handle)
{
	delete Handle;
}

ACPI_CPU_FLAGS Kernel::AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
	KSpinLock* pSpinLock = static_cast<KSpinLock*>(Handle);
	if (!pSpinLock)
		return AE_BAD_PARAMETER;

	return pSpinLock->Acquire();
}

void Kernel::AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
	return static_cast<KSpinLock*>(Handle)->Release(Flags);
}

ACPI_STATUS Kernel::AcpiOsSignal(UINT32 Function, void* Info)
{
	Assert(false);
	return AE_NOT_FOUND;
}

ACPI_STATUS Kernel::AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width)
{
	Assert(false);
	switch (Width)
	{
	case 8:
		*Value = *(UINT8*)Address;
		break;
	case 16:
		*Value = *(UINT16*)Address;
		break;
	case 32:
		*Value = *(UINT32*)Address;
		break;
	case 64:
		*Value = *(UINT64*)Address;
		break;
	default:
		return AE_BAD_VALUE;
	}
	return AE_OK;
}

ACPI_STATUS Kernel::AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
	Assert(false);
	switch (Width)
	{
	case 8:
		*(UINT8*)Address = Value;
		break;
	case 16:
		*(UINT16*)Address = Value;
		break;
	case 32:
		*(UINT32*)Address = Value;
		break;
	case 64:
		*(UINT64*)Address = Value;
		break;
	default:
		return AE_BAD_VALUE;
	}
	return AE_OK;
}

//Should this method only update the portion of the value its reading? (just 8 bits etc)
ACPI_STATUS Kernel::AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width)
{
	switch (Width)
	{
	case 8:
	case 16:
	case 32:
		*Value = x64_read_port(Address, Width);
		break;
	default:
		return AE_BAD_VALUE;
	}

	return AE_OK;
}

ACPI_STATUS Kernel::AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
	switch (Width)
	{
	case 8:
	case 16:
	case 32:
		x64_write_port(Address, Value, Width);
		break;
	default:
		return AE_BAD_VALUE;
	}
	return AE_OK;
}

UINT64 Kernel::AcpiOsGetTimer()
{
	EFI_TIME time;
	m_runtime.GetTime(&time, nullptr);
	uint64_t timer;
	uint16_t year = time.Year;
	uint8_t month = time.Month;
	uint8_t day = time.Day;
	timer = ((uint64_t)(year / 4 - year / 100 + year / 400 + 367 * month / 12 + day) +
		year * 365 - 719499);
	timer *= 24;
	timer += time.Hour;
	timer *= 60;
	timer += time.Minute;
	timer *= 60;
	timer += time.Second;
	timer *= 10000000;	//100 n intervals
	timer += time.Nanosecond / 100;
	return timer;
}

void Kernel::AcpiOsWaitEventsComplete()
{
	Assert(false);
}

ACPI_STATUS Kernel::AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64* Value, UINT32 Width)
{
	//bool result = read_pci_config(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Reg, Width, Value);
	//if (!result)
		//return AE_NOT_IMPLEMENTED;
	Assert(false);
	return AE_OK;
}

ACPI_STATUS Kernel::AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64 Value, UINT32 Width)
{
	//bool result = write_pci_config(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Reg, Width, Value);
	//if (!result)
		//return AE_NOT_IMPLEMENTED;
	Assert(false);
	return AE_OK;
}

ACPI_STATUS Kernel::AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void* Context)
{
	//Assert(false);
	return AE_OK;
}
ACPI_STATUS Kernel::AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler)
{
	//Assert(false);
	return AE_OK;
}
ACPI_STATUS Kernel::AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_PHYSICAL_ADDRESS* NewAddress, UINT32* NewTableLength)
{
	*NewAddress = 0;
	*NewTableLength = 0;
	return AE_OK;
}
