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
extern "C"
{
#include <acpi.h>
}
#include "x64_support.h"


//The one and only
Kernel kernel;

//Kernel Stack - set by x64_main
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[KERNEL_STACK_SIZE] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[KERNEL_STACK_SIZE];

//Kernel Heap
//TODO: It's scratch space now, bring into kernel and make proper heap
KERNEL_PAGE_ALIGN static volatile UINT8 KERNEL_HEAP[KERNEL_HEAP_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static KernelHeap heap((UINT64)KERNEL_HEAP, KERNEL_HEAP_SIZE);

extern "C" void INTERRUPT_HANDLER(InterruptVector vector, PINTERRUPT_FRAME pFrame)
{
	kernel.HandleInterrupt(vector, pFrame);
}

extern "C" void Print(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	kernel.Printf(format, args);
	va_end(args);
}

void* operator new(size_t n)
{
	uintptr_t p = heap.Allocate(n);
	return (void*)p;
}

void operator delete(void* p)
{
	heap.Deallocate((UINT64)p);
}

void operator delete(void* p, size_t n)
{
	heap.Deallocate((UINT64)p);
}

extern "C" void syscall()
{
	Print("Syscall!");
}

void main(LOADER_PARAMS* loader)
{
	//Initialize platform
	x64::Initialize();

	//Initialize kernel
	kernel.Initialize(loader);

	//Idle
	while (true)
		__halt();
}
