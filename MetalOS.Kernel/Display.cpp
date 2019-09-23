#include "Display.h"

Display::Display(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display)
{
	this->_display = display;
}

//PixelBlueGreenRedReserved8BitPerColor
void Display::color_region(UINT32 color)
{
	UINT32* start = (UINT32*)_display->FrameBufferBase;

	//y * width + x
	for (int x = 0; x < 200; x++)
	{
		for (int y = 0; y < 100; y++)
		{
			UINT32* cell = start + (y * _display->Info->PixelsPerScanLine) + x;
			*cell = color;
		}
	}
}
