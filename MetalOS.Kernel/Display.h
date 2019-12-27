#pragma once

#include "Kernel.h"
#include "MetalOS.h"

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include <efilib.h>


class Display
{
public:
	Display(PGRAPHICS_DEVICE pDevice);
	::NO_COPY_OR_ASSIGN(Display);

	void ColorScreen(Color color);
	void ColorRectangle(Color color, Rectangle* region);
	void ColorPixel(Color color, Point2D position);

private:
	PGRAPHICS_DEVICE m_pDevice;
};

