#pragma once

#include <efi.h>
#include <LoaderParams.h>

#include "MetalOS.Kernel.h"
#include "MetalOS.h"

class Display
{
public:
	Display(PEFI_GRAPHICS_DEVICE pDevice);
	::NO_COPY_OR_ASSIGN(Display);

	void ColorScreen(Color color);
	void ColorRectangle(Color color, Rectangle* region);
	void ColorPixel(Color color, Point2D position);

private:
	PEFI_GRAPHICS_DEVICE m_pDevice;
};

