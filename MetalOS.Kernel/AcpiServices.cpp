#include "AcpiServices.h"

#include "msvc.h"
#include "PageFrameAllocator.h"
#include "MetalOS.Kernel.h"
#include "Main.h"
#include "LoadingScreen.h"
#include "PageTables.h"
#include "PageTablesPool.h"
#include <intrin.h>

extern PageFrameAllocator* frameAllocator;
//extern LOADER_PARAMS LoaderParams;

//TODO: AE_NO_MEMORY if we need to

//This file uses a lot of kernel private shit. maybe it should all just be forwarding to kernel calls?

extern "C"
{
#include <acpi.h>
#include "x64.h"
}

ACPI_STATUS AcpiOsInitialize()
{
	return AE_OK;
}

ACPI_STATUS AcpiOsTerminate()
{
	return AE_OK;
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer()
{
	return (ACPI_PHYSICAL_ADDRESS)kernel.GetAcpiRoot();
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
	//Handle unaligned addresses
	size_t pageOffset = PhysicalAddress & PAGE_MASK;
	size_t pageCount = SIZE_TO_PAGES(pageOffset + Length);

	uintptr_t physicalBase = PhysicalAddress & ~PAGE_MASK;
	kernel.GetPageTables()->MapKernelPages(KernelACPIAddress + physicalBase, physicalBase, pageCount);
	
	return (void*)(KernelACPIAddress + physicalBase + pageOffset);
}

void AcpiOsUnmapMemory(void* where, ACPI_SIZE length)
{
	//Trace();
	//TODO
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void* LogicalAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress)
{
	PageTables* current = new PageTables(__readcr3());
	*PhysicalAddress = current->ResolveAddress((uintptr_t)LogicalAddress);

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
	//TODO:
	//Trace();
	return 1;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context)
{
	//TODO
	Assert(false);
	return AE_ERROR;
}

void AcpiOsSleep(UINT64 Milliseconds)
{
	Assert(false);
}

void AcpiOsStall(UINT32 Microseconds)
{
	Assert(false);
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle)
{
	Handle semaphore = kernel.CreateSemaphore(InitialUnits, MaxUnits, "AcpiSemaphore");
	*OutHandle = (void*)semaphore;
	return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
	Assert(false);
	return AE_NO_MEMORY;
}

void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf(const char* Format, ...)
{
	va_list args;
	va_start(args, Format);
	VPrint(Format, args);
	va_end(args);
}

void AcpiOsVprintf(const char* Format, va_list Args)
{
	VPrint(Format, Args);
}

//todo: we get the semaphore twice
ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
	::Handle handle = (::Handle)Handle;
	PSEMAPHORE semaphore = kernel.GetSemaphore(handle);
	if (!semaphore)
		return AE_BAD_PARAMETER;

	if (kernel.WaitSemaphore(handle, Units, Timeout))
		return AE_OK;
	else
		return AE_TIME;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
	::Handle handle = (::Handle)Handle;
	PSEMAPHORE semaphore = kernel.GetSemaphore(handle);
	if (!semaphore)
		return AE_BAD_PARAMETER;
	
	kernel.SignalSemaphore(handle, Units);
	return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle)
{
	*OutHandle = (void*)kernel.CreateSpinlock();
	return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle)
{
	Assert(false);
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
	return kernel.AcquireSpinlock((::Handle)Handle);
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
	kernel.ReleaseSpinlock((::Handle)Handle, Flags);
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info)
{
	Assert(false);
	return AE_NOT_FOUND;
}

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width)
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

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
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
ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width)
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

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
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

UINT64 AcpiOsGetTimer()
{
	return kernel.GetAcpiTimer();
}

void AcpiOsWaitEventsComplete()
{
	Assert(false);
}

ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64* Value, UINT32 Width)
{
	//bool result = read_pci_config(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Reg, Width, Value);
	//if (!result)
		//return AE_NOT_IMPLEMENTED;
	Assert(false);
	return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Reg, UINT64 Value, UINT32 Width)
{
	//bool result = write_pci_config(PciId->Segment, PciId->Bus, PciId->Device, PciId->Function, Reg, Width, Value);
	//if (!result)
		//return AE_NOT_IMPLEMENTED;
	Assert(false);
	return AE_OK;
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void* Context)
{
	//Assert(false);
	return AE_OK;
}
ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler)
{
	//Assert(false);
	return AE_OK;
}
ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_PHYSICAL_ADDRESS* NewAddress, UINT32* NewTableLength)
{
	*NewAddress = 0;
	*NewTableLength = 0;
	return AE_OK;
}
