#pragma once

#include "Common.h"
#include "MetalOS.h"

#include <efi.h>
#include <efilib.h>


class Display
{
public:
	Display() { _display = nullptr;  }
	Display(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display);
	NO_COPY_OR_ASSIGN(Display);

	void ColorScreen(Color color);
	void ColorRectangle(Color color, Rectangle* region);
	void ColorPixel(Color color, Point2D position);

	//THis method needs to go, find a better way to do this in kernel main
	void SetDisplay(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display)
	{
		_display = display;
	}

private:
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* _display;
};

