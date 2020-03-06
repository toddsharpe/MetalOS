#include "msvc.h"
#include <intrin.h>
#include "Main.h"
#include "Kernel.h"
extern "C"
{
#include <acpi.h>
}

//This file is for forwarding global ACPI entries MetalOS.Acpi needs to the kernel.

ACPI_STATUS AcpiOsInitialize()
{
	return kernel.AcpiOsInitialize();
}

ACPI_STATUS AcpiOsTerminate()
{
	return kernel.AcpiOsTerminate();
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer()
{
	return kernel.AcpiOsGetRootPointer();
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* PredefinedObject, ACPI_STRING* NewValue)
{
	return kernel.AcpiOsPredefinedOverride(PredefinedObject, NewValue);
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable)
{
	return kernel.AcpiOsTableOverride(ExistingTable, NewTable);
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{
	return kernel.AcpiOsMapMemory(PhysicalAddress, Length);
}

void AcpiOsUnmapMemory(void* where, ACPI_SIZE length)
{
	return kernel.AcpiOsUnmapMemory(where, length);
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void* LogicalAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress)
{
	return kernel.AcpiOsGetPhysicalAddress(LogicalAddress, PhysicalAddress);
}

void* AcpiOsAllocate(ACPI_SIZE Size)
{
	return kernel.AcpiOsAllocate(Size);
}

void AcpiOsFree(void* Memory)
{
	kernel.AcpiOsFree(Memory);
}

BOOLEAN AcpiOsReadable(void* Memory, ACPI_SIZE Length)
{
	return kernel.AcpiOsReadable(Memory, Length);
}
BOOLEAN AcpiOsWritable(void* Memory, ACPI_SIZE Length)
{
	return kernel.AcpiOsWritable(Memory, Length);
}

ACPI_THREAD_ID AcpiOsGetThreadId()
{
	return kernel.AcpiOsGetThreadId();
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context)
{
	return kernel.AcpiOsExecute(Type, Function, Context);
}

void AcpiOsSleep(UINT64 Milliseconds)
{
	kernel.AcpiOsSleep(Milliseconds);
}

void AcpiOsStall(UINT32 Microseconds)
{
	kernel.AcpiOsStall(Microseconds);
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle)
{
	return kernel.AcpiOsCreateSemaphore(MaxUnits, InitialUnits, OutHandle);
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
	return kernel.AcpiOsDeleteSemaphore(Handle);
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
	kernel.Print(Format, Args);
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
	return kernel.AcpiOsWaitSemaphore(Handle, Units, Timeout);
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
	return kernel.AcpiOsSignalSemaphore(Handle, Units);
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle)
{
	return kernel.AcpiOsCreateLock(OutHandle);
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle)
{
	kernel.AcpiOsDeleteLock(Handle);
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
	return kernel.AcpiOsAcquireLock(Handle);
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
	return kernel.AcpiOsReleaseLock(Handle, Flags);
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info)
{
	return kernel.AcpiOsSignal(Function, Info);
}

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width)
{
	return kernel.AcpiOsReadMemory(Address, Value, Width);
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
	return kernel.AcpiOsWriteMemory(Address, Value, Width);
}

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width)
{
	return kernel.AcpiOsReadPort(Address, Value, Width);
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
	return kernel.AcpiOsWritePort(Address, Value, Width);
}

UINT64 AcpiOsGetTimer()
{
	return kernel.AcpiOsGetTimer();
}

void AcpiOsWaitEventsComplete()
{
	kernel.AcpiOsWaitEventsComplete();
}

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64* Value, UINT32 Width)
{
	return kernel.AcpiOsReadPciConfiguration(PciId, Reg, Value, Width);
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64 Value, UINT32 Width)
{
	return kernel.AcpiOsWritePciConfiguration(PciId, Reg, Value, Width);
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void* Context)
{
	return kernel.AcpiOsInstallInterruptHandler(InterruptLevel, Handler, Context);
}
ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler)
{
	return kernel.AcpiOsRemoveInterruptHandler(InterruptNumber, Handler);
}
ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_PHYSICAL_ADDRESS* NewAddress, UINT32* NewTableLength)
{
	return kernel.AcpiOsPhysicalTableOverride(ExistingTable, NewAddress, NewTableLength);
}