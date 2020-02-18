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
	Print("Allocation 0x%016x (0x%x)", p, n);
	return (void*)p;
}

void operator delete(void* p)
{
	Print("Delete at 0x%16x", p);
	heap.Deallocate((UINT64)p);
}

void operator delete(void* p, size_t n)
{
	Print("Delete at 0x%16x 0x%x", p, n);
	heap.Deallocate((UINT64)p);
}

extern "C" void syscall()
{
	Print("Syscall!");
}

//Need to get virtual pointers to acpi struct
//Find way to keep RuntimeServicesData/Code in kernel address space

void main(LOADER_PARAMS* loader)
{
	//Initialize platform
	x64::Initialize();
	
	//Initialize kernel
	kernel.Initialize(loader);

	//Initialize Frame Allocator
	//frameAllocator = new PageFrameAllocator(*memoryMap);

	//ACPI
	//ACPI_STATUS Status;
	//Status = AcpiInitializeSubsystem();
	//if (ACPI_FAILURE(Status))
	//{
	//	Print("Could not initialize ACPI: %d\n", Status);
	//	__halt();
	//}
	//__halt();
	//AcpiInitializeTables(0, 0, FALSE);

	//System system(loader->ConfigTables, loader->ConfigTableSizes);
	//system.GetInstalledSystemRam();
	//system.DisplayTableIds();
	//system.DisplayAcpi2();

	__halt();
}
