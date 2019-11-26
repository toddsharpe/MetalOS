#pragma once

#include "Kernel.h"
#include "MetalOS.h"

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include <efilib.h>


class Display
{
public:
	Display() { m_pDevice = nullptr;  }
	Display(PGRAPHICS_DEVICE pDevice);
	::NO_COPY_OR_ASSIGN(Display);

	void ColorScreen(Color color);
	void ColorRectangle(Color color, Rectangle* region);
	void ColorPixel(Color color, Point2D position);

	//THis method needs to go, find a better way to do this in kernel main
	void SetDisplay(PGRAPHICS_DEVICE pDevice)
	{
		m_pDevice = pDevice;
	}

private:
	PGRAPHICS_DEVICE m_pDevice;
};

