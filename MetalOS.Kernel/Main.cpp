#include <efi.h>
#include "LoaderParams.h"
#include "Display.h"

extern "C" void main(LOADER_PARAMS* loader)
{
	Display display(&loader->Display);
	display.color_region(0x00FF0000);

	int i = 0;
	while (TRUE)
	{
		i++;
	}
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
