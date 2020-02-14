#include "Display.h"

#include "Main.h"

//Hyper-V
//PixelBlueGreenRedReserved8BitPerColor

Display::Display(EFI_GRAPHICS_DEVICE& device) : m_device(device)
{

}

void Display::ColorScreen(Color color)
{
	Rectangle rectangle;
	rectangle.P1 = { 0, 0 };
	rectangle.P2 = { m_device.HorizontalResolution, m_device.VerticalResolution };
	this->ColorRectangle(color, &rectangle);
}

void Display::ColorRectangle(Color color, Rectangle* region)
{
	uint32_t* start = (uint32_t*)m_device.FrameBufferBase;

	//y * width + x
	//This can definitely be done faster with memory ops
	for (uint32_t x = region->P1.X; x < region->P2.X; x++)
	{
		for (uint32_t y = region->P1.Y; y < region->P2.Y; y++)
		{
			uint32_t* cell = start + (y * m_device.PixelsPerScanLine) + x;
			*(Color*)cell = color;//We can do this because of the static assert in metalos.h
		}
	}
}

void Display::ColorPixel(Color color, Point2D position)
{
	Assert(position.X < m_device.HorizontalResolution);
	Assert(position.Y < m_device.VerticalResolution);

	uint32_t* start = (uint32_t*)m_device.FrameBufferBase;
	uint32_t* cell = start + (position.Y * m_device.PixelsPerScanLine) + position.X;
	*(Color*)cell = color;
}
