#include "msvc.h"
#define GNU_EFI_SETJMP_H
#include <efi.h>
#include "LoaderParams.h"
#include "Display.h"
#include "LoadingScreen.h"
#include "MetalOS.h"
#include "Main.h"
#include "System.h"
#include "MemoryMap.h"
#include "Loader.h"
#include <intrin.h>
#include "CRT.h"
#include "String.h"
#include "PageTables.h"
#include "PageTablesPool.h"
#include "x64.h"
#include "KernelHeap.h"

#include <stdio.h>
#include <string.h>
#include <vector>

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

//Kernel members
Display* display;
LoadingScreen* loading;
PageTablesPool* pagePool;

//Kernel Stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[KERNEL_STACK_SIZE] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[KERNEL_STACK_SIZE];

//Kernel Heap
KERNEL_PAGE_ALIGN static volatile UINT8 KERNEL_HEAP[KERNEL_HEAP_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static KernelHeap heap((UINT64)KERNEL_HEAP, KERNEL_HEAP_SIZE);

KERNEL_GLOBAL_ALIGN static LOADER_PARAMS LoaderParams = { 0 };
KERNEL_GLOBAL_ALIGN static UINT8 EFI_MEMORY_MAP[MemoryMapReservedSize] = { 0 };

extern "C" void INTERRUPT_HANDLER(size_t vector, PINTERRUPT_FRAME pFrame)
{
	loading->WriteLineFormat("ISR: %d, Code: %d, RBP: 0x%16x, RIP: 0x%16x", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP);
	switch (vector)
	{
	case 14:
		loading->WriteLineFormat("CR2: 0x%16x", __readcr2());
		break;
	}
}

extern "C" void Print(const char* format, ...)
{
	char buffer[80];

	va_list ap;

	va_start(ap, format);
	int retval = String::vsprintf(buffer, format, ap);
	buffer[retval] = '\0';
	va_end(ap);

	loading->WriteLine(buffer);
}

void* operator new(size_t n)
{
	//loading->WriteLineFormat("Allocation size 0x%x", n);
	return (void*)heap.Allocate(n);
}

void operator delete(void* p)
{
	//loading->WriteLineFormat("Delete at 0x%16x", p);
	heap.Deallocate((UINT64)p);
}

void operator delete(void* p, size_t n)
{
	//loading->WriteLineFormat("Delete at 0x%16x 0x%x", p, n);
	heap.Deallocate((UINT64)p);
}

//Copy loader params and all recursive structures to kernel memory
extern "C" void main_thunk(LOADER_PARAMS* loader)
{
	CRT::memcpy(&LoaderParams, loader, sizeof(LOADER_PARAMS));
	LoaderParams.MemoryMap = (EFI_MEMORY_DESCRIPTOR*)EFI_MEMORY_MAP;
	CRT::memcpy(EFI_MEMORY_MAP, loader->MemoryMap, loader->MemoryMapSize);

	main(&LoaderParams);
}

void main(LOADER_PARAMS* loader)
{
	//Immediately set up graphics device so we can bugcheck gracefully
	display = new Display(&loader->Display);
	display->ColorScreen(Black);
	loading = new LoadingScreen(*display);

	//Initialize page tables
	pagePool = new PageTablesPool(LoaderParams.PageTablesPoolAddress, LoaderParams.PageTablesPoolPageCount);
	pagePool->SetVirtualAddress(KernelPageTablesPoolAddress);

	//x64 Initialization
	x64::Initialize();

	//Test interrupts
	__debugbreak();

	//Map in kernel to new PT. PageTablesPool has been mapped in by bootloader
	UINT64 ptRoot;
	Assert(pagePool->AllocatePage(&ptRoot));
	PageTables kernelPT(ptRoot);
	kernelPT.SetPool(pagePool);
	kernelPT.SetVirtualOffset(KernelPageTablesPoolAddress - LoaderParams.PageTablesPoolAddress);
	kernelPT.MapKernelPages(KernelBaseAddress, loader->KernelAddress, EFI_SIZE_TO_PAGES(loader->KernelImageSize));
	kernelPT.MapKernelPages(KernelPageTablesPoolAddress, loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	kernelPT.MapKernelPages(KernelGraphicsDeviceAddress, loader->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(loader->Display.FrameBufferSize));
	loader->Display.FrameBufferBase = KernelGraphicsDeviceAddress;
	__writecr3(ptRoot);

	loading->WriteLineFormat("MetalOS.Kernel - Base:0x%16x Size: 0x%x", LoaderParams.KernelAddress, LoaderParams.KernelImageSize);
	loading->WriteLineFormat("LOADER_PARAMS: 0x%16x", loader);
	loading->WriteLineFormat("ConfigTableSizes: %d", loader->ConfigTableSizes);
	loading->WriteLineFormat("MemoryMap: 0x%16x", loader->MemoryMap);
	loading->WriteLineFormat("Display.FrameBufferBase: 0x%16x", loader->Display.FrameBufferBase);

	//Access current EFI memory map
	//Its on its own page so we are fine with resizing
	MemoryMap memoryMap(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapVersion, loader->MemoryMap, PAGE_SIZE);
	memoryMap.ReclaimBootPages();
	memoryMap.MergeConventionalPages();
	memoryMap.DumpMemoryMap();

	__halt();
}

//TODO: fix when merging loading screen and display
void KernelBugcheck(const char* file, const char* line, const char* assert)
{
	//display.ColorScreen(Red);
	
	//loading->ResetX();
	//loading->ResetY();
	loading->WriteLine(file);
	loading->WriteLine(line);
	loading->WriteLine(assert);

	//halt
	__halt();
}

