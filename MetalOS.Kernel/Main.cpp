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
#include "x64_support.h"

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

//Kernel stacks
KERNEL_GLOBAL_ALIGN static volatile UINT8 DOUBLEFAULT_STACK[IST_STACK_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static volatile UINT8 NMI_Stack[IST_STACK_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static volatile UINT8 DEBUG_STACK[IST_STACK_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static volatile UINT8 MCE_STACK[IST_STACK_SIZE] = { 0 };


//Kernel Structures
KERNEL_GLOBAL_ALIGN static TASK_STATE_SEGMENT_64 TSS64 =
{

};

//1 empty, 4 GDTs, and 1 TSS
KERNEL_GLOBAL_ALIGN static KERNEL_GDTS KernelGDT =
{
	{
		{ 0 }, //First entry has to be empty
		// Seg1   Base  type  S   DPL		   P   Seg2   OS      L     DB    4GB   Base
		{ 0xFFFF, 0, 0, 0xA, true, KernelDPL, true, 0xF, false, false, true, true, 0x00 }, //64-bit code Kernel
		{ 0xFFFF, 0, 0, 0x2, true, KernelDPL, true, 0xF, false, false, true, true, 0x00 }, //64-bit data Kernel
		{ 0xFFFF, 0, 0, 0xA, true, UserDPL,	  true, 0xF, false, false, true, true, 0x00 }, //64-bit code User
		{ 0xFFFF, 0, 0, 0x2, true, UserDPL,	  true, 0xF, false, false, true, true, 0x00 }, //64-bit data User
	},
	{
		// Seg1				Base1			Base2							type  S  DPL  P   Seg2	OS      L   DB     4GB   Base3
		sizeof(TSS64) - 1, (UINT16)& TSS64, (UINT8)((UINT64)& TSS64 >> 16), 0x9, false, 0, true, 0, false, false, false, true, (UINT8)((UINT64)& TSS64 >> 24),
		// Base4						Zeroes
		(UINT32)((UINT64)& TSS64 >> 32), 0, 0, 0
	}
};

KERNEL_GLOBAL_ALIGN static IDT_GATE IDT[IDT_COUNT] = { 0 };

//Aligned on word boundary so address load is on correct boundary
__declspec(align(2)) static DESCRIPTOR_TABLE GDTR =
{
	sizeof(KernelGDT) - 1,
	(UINT64)& KernelGDT
};
__declspec(align(2)) static DESCRIPTOR_TABLE IDTR =
{
	sizeof(IDT) - 1,
	(UINT64)IDT
};


//Further initialize structures
void InitializeGlobals()
{
	//Populate TSS structure
	//These assignments need to be consistent with IST_ defines
	//TODO: maybe make an array of addresses to enforce consistency?
	TSS64.IST_1_low = (UINT32)((UINT64)DOUBLEFAULT_STACK);
	TSS64.IST_1_high = (UINT32)(((UINT64)DOUBLEFAULT_STACK) >> 32);
	TSS64.IST_2_low = (UINT32)((UINT64)NMI_Stack);
	TSS64.IST_2_high = (UINT32)(((UINT64)NMI_Stack) >> 32);
	TSS64.IST_3_low = (UINT32)((UINT64)DEBUG_STACK);
	TSS64.IST_3_high = (UINT32)(((UINT64)DEBUG_STACK) >> 32);
	TSS64.IST_4_low = (UINT32)((UINT64)MCE_STACK);
	TSS64.IST_4_high = (UINT32)(((UINT64)MCE_STACK) >> 32);

}

extern "C" void main(LOADER_PARAMS* loader)
{
	//Finish global initialization
	InitializeGlobals();
	
	//Immediately set up graphics device so we can bugcheck gracefully
	display.SetDisplay(&loader->Display);
	display.ColorScreen(Black);

	LoadingScreen localLoading(display);
	loading = &localLoading;

	loading->WriteLineFormat("KernelBase:0x%16x", KernelBaseAddress);
	loading->WriteLineFormat("TSS64- Limit:0x%16x", &TSS64);
	loading->WriteLineFormat("KernelGDT- Limit:0x%08x Base1: 0x%08x", KernelGDT.TssEntry.SegmentLimit1, KernelGDT.TssEntry.BaseAddress1);
	loading->WriteLineFormat("GDTR- Limit:0x%08x Address: 0x%16x", GDTR.Limit, GDTR.BaseAddress);

	//Setup GDT/TSR
	_lgdt(&GDTR);
	x64_ltr((UINT64)&KernelGDT.TssEntry - (UINT64)&KernelGDT.GDT);

	//Set up the page tables
	//PageTablesPool pool(loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	//UINT64 ptRoot;
	//Assert(pool.AllocatePage(&ptRoot));
	//loading->WriteLineFormat("PT - Current: 0x%16x Base: 0x%16x", __readcr3(), ptRoot);

	//Map in kernel, page table pool
	//PageTables kernelPT(__readcr3());
	//PageTables kernelPT(ptRoot);
	//kernelPT.SetPool(&pool);
	//kernelPT.MapKernelPages(KernelBaseAddress, loader->KernelAddress, EFI_SIZE_TO_PAGES(loader->KernelImageSize));
	//kernelPT.MapKernelPages(KernelPageTablesPoolAddress, loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	//pool.SetVirtualAddress(KernelPageTablesPoolAddress);
	//kernelPT.MapKernelPages(KernelGraphicsDeviceAddress, loader->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(loader->Display.FrameBufferSize));
	//loader->Display.FrameBufferBase = KernelGraphicsDeviceAddress;
	//__writecr3(ptRoot);


	loading->WriteLineFormat("MetalOS.Kernel - Base:0x%16x Size: 0x%x", loader->KernelAddress, loader->KernelImageSize);
	loading->WriteLineFormat("LOADER_PARAMS: 0x%16x", loader);
	loading->WriteLineFormat("ConfigTableSizes: %d", loader->ConfigTableSizes);
	loading->WriteLineFormat("MemoryMap: 0x%16x", loader->MemoryMap);
	loading->WriteLineFormat("Display.FrameBufferBase: 0x%16x", loader->Display.FrameBufferBase);

	//Access current EFI memory map
	//Its on its own page so we are fine with resizing
	MemoryMap memoryMap(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapVersion, loader->MemoryMap, PAGE_SIZE);
	//memoryMap.ReclaimBootPages();
	//memoryMap.MergeConventionalPages();

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

