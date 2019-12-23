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

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Display display;
LoadingScreen* loading;
//LOADER_PARAMS* pParams;//TODO: make this extern and have loader get address and fill out struct directly?

//Copy loader params into our own params (possible improvement, is to make this local params a PE/COFF export and have the loader fill that in directly)
//Copy memory map - update efi bootloader to not put it on its own page then?
//Copy configuration?

//Map in kernel
//Map in graphics buffer
//map in page tables pool

//figure out how to get dynamic memory

//Kernel stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[KERNEL_STACK_SIZE] = { 0 };
extern "C"
{
	//Define etern for x64_main
	UINT64 KERNEL_STACK_STOP = (UINT64)& KERNEL_STACK[KERNEL_STACK_SIZE];
}


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
	display.SetDisplay(&loader->Display);
	display.ColorScreen(Black);

	LoadingScreen localLoading(display);
	loading = &localLoading;

	//x64 Initialization
	x64::Initialize();

	//Test interrupts
	__debugbreak();

	//Set up the page tables
	PageTablesPool pool(LoaderParams.PageTablesPoolAddress, LoaderParams.PageTablesPoolPageCount);
	pool.SetVirtualAddress(KernelPageTablesPoolAddress);
	UINT64 ptRoot;
	Assert(pool.AllocatePage(&ptRoot));
	loading->WriteLineFormat("PT - Current: 0x%16x Base: 0x%16x", __readcr3(), ptRoot);

	//Map in kernel to new PT. PageTablesPool has been mapped in by bootloader
	PageTables kernelPT(ptRoot);
	kernelPT.SetPool(&pool);
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
	memoryMap.DumpMemoryMap();

	__halt();
}

//TODO: fix when merging loading screen and display
void KernelBugcheck(const char* assert)
{
	display.ColorScreen(Red);
	
	loading->ResetX();
	loading->ResetY();
	loading->WriteText(assert);

	//halt
	__halt();
}

