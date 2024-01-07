#include <intrin.h>
#include "Main.h"
#include "Kernel/Kernel.h"
#include "Assert.h"
extern "C"
{
#include <acpi.h>
}

//Acpi is single threaded, just stub these out
typedef int semaphore_t;
typedef int spinlock_t;

//This file is for forwarding global ACPI entries MetalOS.Acpi needs to the kernel.

ACPI_STATUS AcpiOsInitialize()
{
	return ACPI_STATUS();
}

ACPI_STATUS AcpiOsTerminate()
{
	NotImplemented();
	return ACPI_STATUS();
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer()
{
	return kernel.AcpiOsGetRootPointer();
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* PredefinedObject, ACPI_STRING* NewValue)
{
	*NewValue = 0;
	return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable)
{
	*NewTable = nullptr;
	return AE_OK;
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{
	return kernel.AcpiOsMapMemory(PhysicalAddress, Length);
}

void AcpiOsUnmapMemory(void* where, ACPI_SIZE length)
{
	UNUSED(where);
	UNUSED(length);
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void* LogicalAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress)
{
	Trace();
	return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size)
{
	return operator new((size_t)Size);
}

void AcpiOsFree(void* Memory)
{
	operator delete(Memory);
}

BOOLEAN AcpiOsReadable(void* Memory, ACPI_SIZE Length)
{
	return TRUE;
}
BOOLEAN AcpiOsWritable(void* Memory, ACPI_SIZE Length)
{
	return TRUE;
}

ACPI_THREAD_ID AcpiOsGetThreadId()
{
	//ACPI is single-threaded, just return 1
	return 1;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context)
{
	NotImplemented();
	return AE_ERROR;
}

void AcpiOsSleep(UINT64 Milliseconds)
{
	NotImplemented();
}

void AcpiOsStall(UINT32 Microseconds)
{
	NotImplemented();
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle)
{
	*OutHandle = new semaphore_t();
	return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
	if (!Handle)
		return AE_BAD_PARAMETER;

	delete Handle;
	return AE_OK;
}

void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf(const char* Format, ...)
{
	va_list args;
	va_start(args, Format);
	AcpiOsVprintf(Format, args);
	va_end(args);
}

void AcpiOsVprintf(const char* Format, va_list Args)
{
	kernel.AcpiOsVprintf(Format, Args);
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
	if (!Handle)
		return AE_BAD_PARAMETER;

	Assert(Units == 1);

	return AE_OK;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
	if (!Handle)
		return AE_BAD_PARAMETER;

	return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle)
{
	*OutHandle = new spinlock_t();
	return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle)
{
	delete Handle;
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
	if (!Handle)
		return AE_BAD_PARAMETER;

	return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
	//NOTE(tsharpe): Nothing to do, spinlocks aren't implemented
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info)
{
	NotImplemented();
	return AE_OK;
}

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width)
{
	NotImplemented();
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

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
	NotImplemented();
	switch (Width)
	{
		case 8:
			*(UINT8*)Address = (UINT8)Value;
			break;
		case 16:
			*(UINT16*)Address = (UINT16)Value;
			break;
		case 32:
			*(UINT32*)Address = (UINT32)Value;
			break;
		case 64:
			*(UINT64*)Address = (UINT64)Value;
			break;
		default:
			return AE_BAD_VALUE;
	}
	return AE_OK;
}

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width)
{
	switch (Width)
	{
		case 8:
		case 16:
		case 32:
			*Value = ArchReadPort((uint32_t)Address, Width);
			break;
		default:
			return AE_BAD_VALUE;
	}

	return AE_OK;
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
	switch (Width)
	{
		case 8:
		case 16:
		case 32:
			ArchWritePort((uint32_t)Address, Value, Width);
			break;
		default:
			return AE_BAD_VALUE;
	}
	return AE_OK;
}

UINT64 AcpiOsGetTimer()
{
	return kernel.AcpiOsGetTimer();
}

void AcpiOsWaitEventsComplete()
{
	NotImplemented();
}

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64* Value, UINT32 Width)
{
	NotImplemented();
	return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64 Value, UINT32 Width)
{
	NotImplemented();
	return AE_OK;
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void* Context)
{
	return AE_OK;
}
ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler)
{
	return AE_OK;
}
ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_PHYSICAL_ADDRESS* NewAddress, UINT32* NewTableLength)
{
	*NewAddress = 0;
	*NewTableLength = 0;
	return AE_OK;
}