#include <efi.h>
#include "LoaderParams.h"

void memset(void* dest, UINT8 value, UINT32 count);

//PixelBlueGreenRedReserved8BitPerColor
void main(LOADER_PARAMS* loader)
{
	//Color screen
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU = loader->Display;
	UINT32 color = 0x00FF0000;
	memset((EFI_PHYSICAL_ADDRESS*)GPU.FrameBufferBase, color, GPU.Info->VerticalResolution * GPU.Info->PixelsPerScanLine);

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
