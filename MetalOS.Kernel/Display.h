#pragma once

#include <efi.h>
#include <LoaderParams.h>

#include "MetalOS.h"
#include "MetalOS.Kernel.h"

class Display
{
public:
	Display(const EFI_GRAPHICS_DEVICE& device);
	Display(const EFI_GRAPHICS_DEVICE& device, const uintptr_t virtualAddress);
	::NO_COPY_OR_ASSIGN(Display);

	void ColorScreen(const Color color) const;
	void ColorRectangle(const Color color, const Rectangle& region) const;

	void WriteFrame(const Rectangle& rect, const Buffer& buffer);
	void WriteFrameGrayscale(const Rectangle& rect, const Buffer& buffer);

	void SetPixel(const Color color, const Point2D& position) const;
	Color GetPixel(const Point2D& position) const;

	uint32_t GetWidth() const
	{
		return m_device.HorizontalResolution;
	}

	uint32_t GetHeight() const
	{
		return m_device.VerticalResolution;
	}

	uintptr_t GetBuffer() const
	{
		return m_address;
	}

private:
	const uintptr_t m_address;
	const EFI_GRAPHICS_DEVICE m_device;
};

