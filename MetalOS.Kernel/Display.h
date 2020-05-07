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
	Display(EFI_GRAPHICS_DEVICE& device, uintptr_t virtualAddress);
	::NO_COPY_OR_ASSIGN(Display);

	void ColorScreen(Color color);
	void ColorRectangle(Color color, Rectangle* region);

	void SetPixel(Color color, Point2D position);
	Color GetPixel(Point2D position);

	uint32_t GetWidth() const
	{
		return m_device.HorizontalResolution;
	}

	uint32_t GetHeight() const
	{
		return m_device.VerticalResolution;
	}

	uintptr_t Buffer() const
	{
		return m_address;
	}

private:
	uintptr_t m_address;
	EFI_GRAPHICS_DEVICE m_device;
};

