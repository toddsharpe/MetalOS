#include <efi.h>
#include "LoaderParams.h"
#include "Display.h"

void memset(void* dest, UINT8 value, UINT32 count);

void color_screen(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display, UINT32 color);


//PixelBlueGreenRedReserved8BitPerColor
void main(LOADER_PARAMS* loader)
{
	//Color screen
	//EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU = loader->Display;
	//UINT32 color = 0x00FF0000;
	//memset((EFI_PHYSICAL_ADDRESS*)GPU.FrameBufferBase, color, GPU.Info->VerticalResolution * GPU.Info->PixelsPerScanLine);

	//color_screen(&loader->Display, color);

	Display display(loader->Display);

	int i = 0;
	while (TRUE)
	{
		i++;
	}
}

void color_screen(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display, UINT32 color)
{
	UINT32* start = (UINT32*)display->FrameBufferBase;
	
	//y * width + x
	for (int x = 0; x < 200; x++)
	{
		for (int y = 0; y < 100; y++)
		{
			UINT32* cell = start + (y * display->Info->PixelsPerScanLine) + x;
			*cell = color;
		}
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
