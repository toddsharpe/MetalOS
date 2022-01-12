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
	Rectangle rectangle = {};
	rectangle.X = 0;
	rectangle.Y = 0;
	rectangle.Width = m_device.HorizontalResolution;
	rectangle.Height = m_device.VerticalResolution;
	this->ColorRectangle(color, rectangle);
}

void* setpixels(void* _Dst, Color _Val, size_t _Size)
{
	Color* dest_32 = (Color*)_Dst;
	for (size_t i = 0; i < _Size; i++)
	{
		*dest_32 = (Color)_Val;
		dest_32++;
	}

	return _Dst;
}

//This can definitely be done faster with memory ops
//Rewrite to iterate over y, and then call memset32
void Display::ColorRectangle(const Color color, const Rectangle& region) const
{
	uint32_t* start = (uint32_t*)m_address;

	for (size_t y = 0; y < region.Height; y++)
	{
		if (y + region.Y >= m_device.VerticalResolution)
			break;

		//Write row
		uint32_t* dst = (uint32_t*)m_address + (region.Y + y) * m_device.PixelsPerScanLine + region.X;
		size_t count = std::min(region.Width, m_device.HorizontalResolution - region.X);
		setpixels(dst, color, count);
	}
}

void* memset32(void* _Dst, int _Val, size_t _Size)
{
	uint32_t* dest_32 = (uint32_t*)_Dst;
	for (size_t i = 0; i < _Size; i++)
	{
		*dest_32 = (uint32_t)_Val;
		dest_32++;
	}

	return _Dst;
}

void Display::WriteFrame(const Rectangle& rect, const Buffer& buffer)
{
	for (size_t y = 0; y < rect.Height; y++)
	{
		if (y + rect.Y >= GetHeight())
			break;
		
		//Write row
		uint32_t* dst = (uint32_t*)m_address + (rect.Y + y) * m_device.PixelsPerScanLine + rect.X;
		const uint32_t* src = (uint32_t*)buffer.Data + y * rect.Width;
		size_t count = std::min(rect.Width, GetWidth() - rect.X);

		memcpy(dst, src, count * sizeof(Color));
	}
}

void Display::WriteFrameGrayscale(const Rectangle& rect, const Buffer& buffer)
{
	for (size_t y = 0; y < rect.Height; y++)
	{
		if (y + rect.Y >= GetHeight())
			break;

		//Write row
		Color* dst = (Color*)m_address + (rect.Y + y) * m_device.PixelsPerScanLine + rect.X;
		const Color* src = (Color*)buffer.Data + y * rect.Width;
		size_t count = std::min(rect.Width, GetWidth() - rect.X);

		for (size_t x = 0; x < count; x++)
		{
			size_t sum = src->Blue + src->Green + src->Red;
			uint8_t average = sum / 3;
			
			dst->Blue = average;
			dst->Green = average;
			dst->Red = average;
			dst->Reserved = src->Reserved;

			dst++;
			src++;
		}

		//memcpy(dst, src, count * sizeof(Color));
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
