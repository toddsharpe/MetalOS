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
#include "PageTables.h"
#include "PageTablesPool.h"
#include <intrin.h>

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

extern "C" void main(LOADER_PARAMS* loader)
{
	//Immediately set up graphics device so we can bugcheck gracefully
	display.SetDisplay(&loader->Display);
	display.ColorScreen(Black);

	//Set up the page tables
	PageTablesPool pool(loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	pool.DeallocatePage(loader->PageTablesPoolAddress + EFI_PAGE_SIZE);//This removes the one page the bootloader needed
	
	//Map in kernel, page table pool
	PageTables kernelPT(__readcr3());
	kernelPT.MapKernelPages(KernelBaseAddress, loader->KernelAddress, EFI_SIZE_TO_PAGES(loader->KernelImageSize));
	kernelPT.MapKernelPages(KernelPageTablesPoolAddress, loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	pool.SetVirtualAddress(KernelPageTablesPoolAddress);
	kernelPT.MapKernelPages(KernelGraphicsDeviceAddress, loader->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(loader->Display.FrameBufferSize));
	loader->Display.FrameBufferBase = KernelGraphicsDeviceAddress;

	LoadingScreen localLoading(display);
	loading = &localLoading;
	loading->WriteLineFormat("MetalOS.Kernel - Base:0x%16x Size: 0x%x S2: 0x%x", loader->KernelAddress, loader->KernelImageSize);
	loading->WriteLineFormat("LOADER_PARAMS: 0x%16x", loader);
	loading->WriteLineFormat("ConfigTableSizes: %d", loader->ConfigTableSizes);
	loading->WriteLineFormat("MemoryMap: 0x%16x", loader->MemoryMap);
	loading->WriteLineFormat("Display.FrameBufferBase: 0x%16x", loader->Display.FrameBufferBase);

	//Access current EFI memory map
	//Its on its own page so we are fine with resizing
	MemoryMap memoryMap(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapVersion, loader->MemoryMap, PAGE_SIZE);
	memoryMap.ReclaimBootPages();
	memoryMap.MergeConventionalPages();

	//Allocate pages for PageTablesPool
	//EFI_PHYSICAL_ADDRESS pageTables = memoryMap.AllocatePages(ReservedPageTablePages);
	//CRT::memset((void*)pageTables, 0, ReservedPageTableSpace);

	//Construct pool and map in page tables
	//PageTablesPool pool(pageTables, ReservedPageTablePages);//This needs to be in persistent memory
	//UINT64 root = pool.AllocatePage();

	//PageTables newTables(root);
	//newTables.SetPool(&pool);
	//newTables.MapKernelPages((UINT64)(KernelStart + pageTables), pageTables, ReservedPageTablePages); //// WTFFFFFFFFFFF

	//UINT64 virtualAddress = newTables.ResolveAddress(pageTables);
	//loading->WriteLineFormat("virtualAddress: 0x%16x", virtualAddress);


	memoryMap.DumpMemoryMap();

	//Reconstruct our map there
	//memoryMap = MemoryMap(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapVersion, (EFI_MEMORY_DESCRIPTOR*)address, pages * EFI_PAGE_SIZE);
	
	//memoryMap.AllocatePages(pages);//not using return value?

	//memoryMap.ReclaimBootPages();

	//memoryMap.MergeConventionalPages();

	//Carve out area of memory map for page table region

	//Make new page table in pool
	//map memory map, map pool, map kernel

	//

	//memoryMap.DumpMemoryMap();


	//Map in page table pool


	//Map a region of physical pages to the highest possible address to use for page tables
	//PageTables current(__readcr3() & ~0xFFF);
	//current.SetPool(&pool);

	//System system(loader->ConfigTables, loader->ConfigTableSizes);

	//system.GetInstalledSystemRam();
	//bool b = system.IsPagingEnabled();
	//loading->WriteLineFormat("IsPagingEnabled: %d", b);
	//UINT64 phys = system.ResolveAddress(loader->BaseAddress);
	//UINT64 phys = system.ResolveAddress((UINT64)loader->Display.Info);
	//loading->WriteLineFormat("Resolved: 0x%16x", phys);

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

