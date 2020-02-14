#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include <LoaderParams.h>

#include "MetalOS.h"
#include "MetalOS.Kernel.h"

class Display
{
public:
	Display(EFI_GRAPHICS_DEVICE& device);
	::NO_COPY_OR_ASSIGN(Display);

	void ColorScreen(Color color);
	void ColorRectangle(Color color, Rectangle* region);
	void ColorPixel(Color color, Point2D position);

private:
	EFI_GRAPHICS_DEVICE m_device;
};

