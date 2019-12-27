#include "Display.h"
#include "MetalOS.h"
#include "Main.h"

//Hyper-V
//PixelBlueGreenRedReserved8BitPerColor

Display::Display(PGRAPHICS_DEVICE pDevice) : m_pDevice(pDevice)
{

}

void Display::ColorScreen(Color color)
{
	Rectangle rectangle;
	rectangle.P1 = { 0, 0 };
	rectangle.P2 = { m_pDevice->HorizontalResolution, m_pDevice->VerticalResolution };
	this->ColorRectangle(color, &rectangle);
}

void Display::ColorRectangle(Color color, Rectangle* region)
{
	UINT32* start = (UINT32*)m_pDevice->FrameBufferBase;

	//y * width + x
	//This can definitely be done faster with memory ops
	for (UINT32 x = region->P1.X; x < region->P2.X; x++)
	{
		for (UINT32 y = region->P1.Y; y < region->P2.Y; y++)
		{
			UINT32* cell = start + (y * m_pDevice->PixelsPerScanLine) + x;
			*(Color*)cell = color;//We can do this because of the static assert in metalos.h
		}
	}
}

void Display::ColorPixel(Color color, Point2D position)
{
	Assert(position.X < m_pDevice->HorizontalResolution);
	Assert(position.Y < m_pDevice->VerticalResolution);

	UINT32* start = (UINT32*)m_pDevice->FrameBufferBase;
	UINT32* cell = start + (position.Y * m_pDevice->PixelsPerScanLine) + position.X;
	*(Color*)cell = color;
}
