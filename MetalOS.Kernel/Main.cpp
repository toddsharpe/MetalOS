#include <efi.h>
#include "LoaderParams.h"
#include "Display.h"
#include "LoadingScreen.h"
#include "MetalOS.h"
#include "Main.h"
#include "System.h"

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Display display;
LOADER_PARAMS* pParams;

extern "C" void main(LOADER_PARAMS* loader)
{
	pParams = loader;
	
	display.SetDisplay(&loader->Display);
	display.ColorScreen(Black);

	LoadingScreen loading(display);
	loading.WriteLine("MetalOS.Kernel");
	loading.WriteLineFormat("ConfigTableSizes: %d", loader->ConfigTableSizes);

	

	System system(loader->ConfigTables, loader->ConfigTableSizes);

	int i = 0;
	while (TRUE)
	{
		i++;
	}
}

//TODO: fix when merging loading screen and display
void KernelBugcheck(const char* assert)
{
	display.ColorScreen(Red);
	
	LoadingScreen loading(display);
	loading.WriteText(assert);

	//halt
}

void memset(void* dest, UINT8 value, UINT32 count)
{
	UINT8* dest_8 = (UINT8*)dest;

	while (count > 0)
	{
		*dest_8 = value;
		dest_8++;
		count--;
	}
}
