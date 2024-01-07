#pragma once

#include <efi.h>
#include "LoaderParams.h"
#include <Graphics/Framebuffer.h>
#include "Kernel/MetalOS.Kernel.h"

//TODO(tsharpe): This class is just a Preallocated framebuffer. Move to Graphics.
class EfiDisplay : public Graphics::FrameBuffer
{
public:
	EfiDisplay(void* const address, const size_t height, const size_t width);

	virtual size_t GetHeight() const override;
	virtual size_t GetWidth() const override;
	virtual Graphics::Color* GetBuffer() override;

	//Write buffer at once
	void Write(Graphics::FrameBuffer& framebuffer);

private:
	Graphics::Color* const m_buffer;
	const size_t m_height;
	const size_t m_width;

	::NO_COPY_OR_ASSIGN(EfiDisplay);
};
