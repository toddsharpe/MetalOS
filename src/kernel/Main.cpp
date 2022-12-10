#include "Main.h"

#include "Kernel.h"
#include "Assert.h"
#include "BootHeap.h"

//The one and only, statically allocated
Kernel kernel;

//Init Stack - set by x64_main
//TODO: reclaim
static constexpr size_t InitStackSize = PAGE_SIZE << 8; //16kb Init Stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[InitStackSize] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[InitStackSize];

//Boot Heap
static constexpr size_t BootHeapSize = PAGE_SIZE << 12; //4MB Boot Heap
KERNEL_PAGE_ALIGN static volatile UINT8 BOOT_HEAP[BootHeapSize] = { 0 };
KERNEL_GLOBAL_ALIGN BootHeap bootHeap((void*)BOOT_HEAP, BootHeapSize);

extern "C" void INTERRUPT_HANDLER(InterruptVector vector, PINTERRUPT_FRAME pFrame)
{
	kernel.HandleInterrupt(vector, pFrame);
}

extern "C" int __cdecl atexit(void(__cdecl*)(void))
{
	return 0;
}

void Printf(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	kernel.Printf(format, args);
	va_end(args);
}

void Printf(const char* format, va_list args)
{
	kernel.Printf(format, args);
}

void Bugcheck(const char* file, const char* line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	kernel.Bugcheck(file, line, format, args);
	va_end(args);
}

void* operator new(const size_t n)
{
	uintptr_t callerAddress = (uintptr_t)_ReturnAddress();
	if (kernel.IsHeapInitialized())
		return kernel.Allocate(n, callerAddress);
	else
		return (void*)bootHeap.Allocate(n);
}

void* operator new[](size_t n)
{
	uintptr_t callerAddress = (uintptr_t)_ReturnAddress();
	if (kernel.IsHeapInitialized())
		return kernel.Allocate(n, callerAddress);
	else
		return (void*)bootHeap.Allocate(n);
}

void operator delete(void* const p)
{
	if (kernel.IsHeapInitialized())
		kernel.Deallocate(p);
	else
		bootHeap.Deallocate(p);
}

void operator delete[](void* const p)
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

void* __cdecl malloc(size_t size)
{
	uintptr_t callerAddress = (uintptr_t)_ReturnAddress();
	Assert(kernel.IsHeapInitialized());
	return kernel.Allocate(size, callerAddress);
}

void __cdecl free(void* ptr)
{
	Assert(kernel.IsHeapInitialized());
	return kernel.Deallocate(ptr);
}

extern "C" uint64_t SYSTEMCALL_HANDLER(SystemCallFrame* frame)
{
	return kernel.Syscall(frame);
}

extern MetalOSLibrary BootLibrary;
int main(LOADER_PARAMS* loader)
{
	//BootLibrary.DebugPrint = &Printf;
	//BootLibrary.AssertPrint = &Printf;
	
	//Initialize kernel
	kernel.Initialize(loader);

	//Load init process
	kernel.KeCreateProcess(std::string("init.exe"));

	//Exit init thread
	kernel.KeExitThread();

	Assert(false);
}
