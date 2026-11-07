#pragma once

#include "Lib/System.h"

struct KFramebufferInfo
{
	paddr_t Base;
	size_t Size;
	uint32_t Width;
	uint32_t Height;
	uint32_t Pitch; //pixels per scan line (>= Width)
};

KFramebufferInfo KeGetFramebuffer(); //The loader's linear framebuffer, for a process (the usermode WM) to map
