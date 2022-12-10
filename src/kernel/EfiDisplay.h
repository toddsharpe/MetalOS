#pragma once

#include <efi.h>
#include <kernel/LoaderParams.h>
#include <Graphics/FrameBuffer.h>
#include "MetalOS.Kernel.h"

class EfiDisplay : public Graphics::FrameBuffer
{
public:
	EfiDisplay();

	void Init(void* address, const EFI_GRAPHICS_DEVICE& device);
	void Write(Graphics::FrameBuffer& framebuffer);

	virtual size_t GetHeight() const override;
	virtual size_t GetWidth() const override;
	virtual Graphics::Color* GetBuffer() override;

private:
	void* m_address;
	EFI_GRAPHICS_DEVICE m_device;

	::NO_COPY_OR_ASSIGN(EfiDisplay);
};

