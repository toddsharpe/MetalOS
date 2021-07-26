#include "Main.h"

#include "Kernel.h"
#include "Assert.h"
#include "BootHeap.h"

//The one and only, statically allocated
Kernel kernel;

//Init Stack - set by x64_main
//TODO: reclaim
static constexpr size_t InitStackSize = PAGE_SIZE << 2; //16kb Init Stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[InitStackSize] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[InitStackSize];

//Boot Heap
static constexpr size_t BootHeapSize = PAGE_SIZE << 8; //1MB Boot Heap
KERNEL_PAGE_ALIGN static volatile UINT8 BOOT_HEAP[BootHeapSize] = { 0 };
KERNEL_GLOBAL_ALIGN BootHeap bootHeap((void*)BOOT_HEAP, BootHeapSize);

extern "C" void INTERRUPT_HANDLER(InterruptVector vector, PINTERRUPT_FRAME pFrame)
{
	kernel.HandleInterrupt(vector, pFrame);
}

void Print(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	kernel.Printf(format, args);
	va_end(args);
}

void Print(const char* format, va_list args)
{
	kernel.Printf(format, args);
}

void* operator new(size_t n)
{
	uintptr_t callerAddress = (uintptr_t)_ReturnAddress();
	if (kernel.IsHeapInitialized())
		return kernel.Allocate(n, callerAddress);
	else
		return (void*)bootHeap.Allocate(n);
}

void operator delete(void* p)
{
	if (kernel.IsHeapInitialized())
		kernel.Deallocate(p);
	else
		bootHeap.Deallocate(p);
}

void operator delete(void* p, size_t n)
{
	if (kernel.IsHeapInitialized())
		kernel.Deallocate(p);
	else
		bootHeap.Deallocate(p);
}

void* malloc(size_t size)
{
	uintptr_t callerAddress = (uintptr_t)_ReturnAddress();
	Assert(kernel.IsHeapInitialized());
	return kernel.Allocate(size, callerAddress);
}

void free(void* ptr)
{
	Assert(kernel.IsHeapInitialized());
	return kernel.Deallocate(ptr);
}

extern "C" uint64_t SYSTEMCALL_HANDLER(SystemCallFrame* frame)
{
	return kernel.Syscall(frame);
}

extern "C" void Bugcheck(const char* file, const char* line, const char* assert)
{
	kernel.Bugcheck(file, line, assert);
}

void main(LOADER_PARAMS* loader)
{
	//Initialize kernel
	kernel.Initialize(loader);

	//Load process
	//kernel.CreateProcess("fire.exe");
	kernel.CreateProcess("doom.exe");
	//kernel.CreateProcess("shell.exe");

	//Exit init thread
	kernel.ExitKernelThread();

	Assert(false);
}
