#define GNU_EFI_SETJMP_H
#include <efi.h>
#include "LoaderParams.h"
#include "Display.h"
#include "LoadingScreen.h"
#include "MetalOS.h"
#include "Main.h"
#include "System.h"
#include "Memory.h"
#include "Loader.h"
#include <intrin.h>

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Display display;
LoadingScreen* loading;
LOADER_PARAMS* pParams;

extern "C" void main(LOADER_PARAMS* loader)
{
	pParams = loader;

	display.SetDisplay(&loader->Display);
	display.ColorScreen(Black);

	UINT32 kernelPages = Loader::GetImageSize((void*)loader->BaseAddress) >> EFI_PAGE_SHIFT;

	LoadingScreen localLoading(display);
	loading = &localLoading;
	loading->WriteLineFormat("MetalOS.Kernel - Base:0x%08x Size: 0x%x", loader->BaseAddress, kernelPages);
	loading->WriteLineFormat("ConfigTableSizes: %d", loader->ConfigTableSizes);

	Memory memory(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapVersion, loader->MemoryMap,
		loader->BaseAddress, kernelPages);
	//memory.ReclaimBootPages();
	//memory.MergeConventionalPages();
	//memory.DumpMemoryMap();


	System system(loader->ConfigTables, loader->ConfigTableSizes);
	//system.GetInstalledSystemRam();
	bool b = system.IsPagingEnabled();
	loading->WriteLineFormat("IsPagingEnabled: %d", b);
	//UINT64 phys = system.ResolveAddress(loader->BaseAddress);
	UINT64 phys = system.ResolveAddress((UINT64)loader->Display.Info);
	loading->WriteLineFormat("Resolved: 0x%16x", phys);

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

