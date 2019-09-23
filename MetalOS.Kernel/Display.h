#pragma once

#include "Common.h"

#include <efi.h>
#include <efilib.h>


class Display
{
public:
	Display(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display);
	NO_COPY_OR_ASSIGN(Display);

	void color_region(UINT32 color);

private:
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* _display;
};

