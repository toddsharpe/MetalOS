#include "Main.h"

#include "msvc.h"
#include <efi.h>
#include <LoaderParams.h>
#include <crt_stdio.h>
#include <vector>
#include <intrin.h>
#include "MetalOS.Kernel.h"
#include "MetalOS.h"
#include "System.h"
#include "MemoryMap.h"
#include "PageTables.h"
#include "PageTablesPool.h"
#include "x64.h"
#include "KernelHeap.h"
#include "Bitvector.h"
#include "PageFrameAllocator.h"

extern "C"
{
#include <acpi.h>
}

//Kernel members
PageFrameAllocator* frameAllocator;

Kernel kernel;

//Kernel Stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[KERNEL_STACK_SIZE] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[KERNEL_STACK_SIZE];

//Kernel Heap
KERNEL_PAGE_ALIGN static volatile UINT8 KERNEL_HEAP[KERNEL_HEAP_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static KernelHeap heap((UINT64)KERNEL_HEAP, KERNEL_HEAP_SIZE);

extern "C" void INTERRUPT_HANDLER(size_t vector, PINTERRUPT_FRAME pFrame)
{
	kernel.HandleInterrupt(vector, pFrame);
}

extern "C" void Print(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	kernel.Print(format, args);
	va_end(args);
}

extern "C" void VPrint(const char* format, va_list args)
{
	kernel.Print(format, args);
}

void* operator new(size_t n)
{
	uintptr_t p = heap.Allocate(n);
	//Print("Allocation 0x%016x (0x%x)", p, n);
	return (void*)p;
}

void operator delete(void* p)
{
	//Print("Delete at 0x%16x", p);
	heap.Deallocate((UINT64)p);
}

void operator delete(void* p, size_t n)
{
	//Print("Delete at 0x%16x 0x%x", p, n);
	heap.Deallocate((UINT64)p);
}

extern "C" void syscall()
{
	Print("Syscall!");
}

//Need to get virtual pointers to acpi struct
//Find way to keep RuntimeServicesData/Code in kernel address space

ACPI_STATUS PrintDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue);
void main(LOADER_PARAMS* loader)
{
	//Initialize platform
	x64::Initialize();
	
	//Initialize kernel
	kernel.Initialize(loader);

	//Initialize Frame Allocator
	//frameAllocator = new PageFrameAllocator(*memoryMap);

	//ACPI - ACPI CA Page 41
	ACPI_STATUS Status;
	Status = AcpiInitializeSubsystem();
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeSubsystem: %d\n", Status);
		__halt();
	}
	Print("AcpiInitializeSubsystem\n");

	//AcpiReallocateRootTable ?

	Status = AcpiInitializeTables(nullptr, 16, FALSE);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeTables: %d\n", Status);
		__halt();
	}
	Print("AcpiInitializeTables\n");

	//TODO: notify handlers

	Status = AcpiLoadTables();
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiLoadTables: %d\n", Status);
		__halt();
	}
	Print("AcpiLoadTables\n");

	//Local handlers should be installed here

	Status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiEnableSubsystem: %d\n", Status);
		__halt();
	}
	Print("AcpiEnableSubsystem\n");



	Status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeObjects: %d\n", Status);
		__halt();
	}
	Print("AcpiInitializeObjects\n");

	//Attempt to walk namespace
	Status = AcpiGetDevices(NULL, PrintDevice, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiEnableSubsystem: %d\n", Status);
		__halt();
	}
	Print("AcpiGetDevices\n");

	Print("ACPI Finished\n");

	//System system(loader->ConfigTables, loader->ConfigTableSizes);
	//system.GetInstalledSystemRam();
	//system.DisplayTableIds();
	//system.DisplayAcpi2();

	__halt();
}

//http://quest.bu.edu/qxr/source/kernel/smp/acpi.c#0433
ACPI_STATUS PrintDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue)
{
	ACPI_STATUS Status;
	char Buffer[256];
	ACPI_BUFFER Path = { sizeof(Buffer), &Buffer };
	
	Status = AcpiGetName(Object, ACPI_FULL_PATHNAME, &Path);
	if (ACPI_SUCCESS(Status))
		Print("%-16s: ", Path.Pointer);
	else
		Print("<Name>: ");

	ACPI_DEVICE_INFO* Info;
	Status = AcpiGetObjectInfo(Object, &Info);
	if (ACPI_SUCCESS(Status))
	{
		Print("F: 0x%02x V: 0x%04x T: 0x%08x\n", Info->Flags, Info->Valid, Info->Type);

		if (Info->Valid & ACPI_VALID_ADR)
			Print("    ADR: 0x%x\n", Info->Address);

		if (Info->Valid & ACPI_VALID_HID)
			Print("    HID: %s\n", Info->HardwareId.String);

		if (Info->Valid & ACPI_VALID_UID)
			Print("    UID: %s\n", Info->UniqueId.String);

		if (Info->Valid & ACPI_VALID_CID)
			for (size_t i = 0; i < Info->CompatibleIdList.Count; i++)
			{
				Print("    CID: %s\n", Info->CompatibleIdList.Ids[i].String);
			}
	}

	return AE_OK;
}


