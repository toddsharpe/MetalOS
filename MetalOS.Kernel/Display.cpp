#include "Display.h"

#include <algorithm>
#include "Main.h"

//Hyper-V
//PixelBlueGreenRedReserved8BitPerColor

Display::Display(EFI_GRAPHICS_DEVICE& device) : Display(device, device.FrameBufferBase)
{

}

Display::Display(EFI_GRAPHICS_DEVICE& device, uintptr_t virtualAddress) : m_address(virtualAddress), m_device(device)
{

}

void Display::ColorScreen(Color color)
{
	Rectangle rectangle;
	rectangle.P1 = { 0, 0 };
	rectangle.P2 = { m_device.HorizontalResolution, m_device.VerticalResolution };
	this->ColorRectangle(color, &rectangle);
}

//This can definitely be done faster with memory ops
void Display::ColorRectangle(Color color, Rectangle* region)
{
	uint32_t* start = (uint32_t*)m_address;

	//y * width + x
	for (uint32_t x = region->P1.X; x < region->P2.X; x++)
	{
		for (uint32_t y = region->P1.Y; y < region->P2.Y; y++)
		{
			std::clamp(x, 0U, m_device.HorizontalResolution - 1);
			std::clamp(y, 0U, m_device.VerticalResolution - 1);
			uint32_t* cell = start + ((size_t)y * m_device.PixelsPerScanLine) + x;
			*(Color*)cell = color;//We can do this because of the static assert in metalos.h
		}
	}
}

void Display::SetPixel(Color color, Point2D position)
{
	Assert(position.X < m_device.HorizontalResolution);
	Assert(position.Y < m_device.VerticalResolution);

	uint32_t* start = (uint32_t*)m_address;
	uint32_t* cell = start + ((size_t)position.Y * m_device.PixelsPerScanLine) + position.X;
	*(Color*)cell = color;
}

Color Display::GetPixel(Point2D position)
{
	Assert(position.X < m_device.HorizontalResolution);
	Assert(position.Y < m_device.VerticalResolution);

	uint32_t* start = (uint32_t*)m_address;
	uint32_t* cell = start + ((size_t)position.Y * m_device.PixelsPerScanLine) + position.X;
	return *(Color*)cell;
}
