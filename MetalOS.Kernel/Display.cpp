#include "Display.h"
#include "MetalOS.h"

//Hyper-V
//PixelBlueGreenRedReserved8BitPerColor

Display::Display(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display)
{
	this->_display = display;
}

void Display::ColorScreen(Color color)
{
	Rectangle rectangle;
	rectangle.P1 = { 0, 0 };
	rectangle.P2 = { _display->Info->HorizontalResolution, _display->Info->VerticalResolution };
	this->ColorRectangle(color, &rectangle);
}

void Display::ColorRectangle(Color color, Rectangle* region)
{
	UINT32* start = (UINT32*)_display->FrameBufferBase;
	Color use = { 0 };
	use.Red = 0xFF;
	//y * width + x
	//This can definitely be done faster with memory ops
	for (UINT32 x = region->P1.X; x < region->P2.X; x++)
	{
		for (UINT32 y = region->P1.Y; y < region->P2.Y; y++)
		{
			UINT32* cell = start + (y * _display->Info->PixelsPerScanLine) + x;
			*(Color*)cell = color;//We can do this because of the static assert in metalos.h
		}
	}
}

