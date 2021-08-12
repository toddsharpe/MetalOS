#include "Kernel.h"
#include "Assert.h"
#include <algorithm>

#include "Display.h"

//Hyper-V
//PixelBlueGreenRedReserved8BitPerColor

Display::Display(const EFI_GRAPHICS_DEVICE& device) :
	Display(device, device.FrameBufferBase)
{

}

Display::Display(const EFI_GRAPHICS_DEVICE& device, const uintptr_t virtualAddress) :
	m_address(virtualAddress),
	m_device(device)
{

}

void Display::ColorScreen(const Color color) const
{
	Rectangle rectangle;
	rectangle.P1 = { 0, 0 };
	rectangle.P2 = { m_device.HorizontalResolution, m_device.VerticalResolution };
	this->ColorRectangle(color, rectangle);
}

//This can definitely be done faster with memory ops
void Display::ColorRectangle(const Color color, const Rectangle& region) const
{
	uint32_t* start = (uint32_t*)m_address;

	//y * width + x
	for (uint32_t x = region.P1.X; x < region.P2.X; x++)
	{
		for (uint32_t y = region.P1.Y; y < region.P2.Y; y++)
		{
			x = std::clamp(x, 0U, m_device.HorizontalResolution - 1);
			y = std::clamp(y, 0U, m_device.VerticalResolution - 1);
			uint32_t* cell = start + ((size_t)y * m_device.PixelsPerScanLine) + x;
			*(Color*)cell = color;//We can do this because of the static assert in metalos.h
		}
	}
}

void Display::SetPixel(const Color color, const Point2D& position) const
{
	Assert(position.X < m_device.HorizontalResolution);
	Assert(position.Y < m_device.VerticalResolution);

	uint32_t* start = (uint32_t*)m_address;
	uint32_t* cell = start + ((size_t)position.Y * m_device.PixelsPerScanLine) + position.X;
	*(Color*)cell = color;
}

Color Display::GetPixel(const Point2D& position) const
{
	Assert(position.X < m_device.HorizontalResolution);
	Assert(position.Y < m_device.VerticalResolution);

	uint32_t* start = (uint32_t*)m_address;
	uint32_t* cell = start + ((size_t)position.Y * m_device.PixelsPerScanLine) + position.X;
	return *(Color*)cell;
}
