#include "Assert.h"
#include <algorithm>

#include "EfiDisplay.h"

EfiDisplay::EfiDisplay() : 
	Graphics::FrameBuffer(),
	m_address(),
	m_device()
{
	
}

void EfiDisplay::Init(void* address, const EFI_GRAPHICS_DEVICE& device)
{
	m_address = address;
	m_device = device;
}

void EfiDisplay::Write(Graphics::FrameBuffer& framebuffer)
{
	const size_t bufferSize = framebuffer.GetHeight() * framebuffer.GetWidth() * sizeof(Graphics::Color);
	memcpy(m_address, framebuffer.GetBuffer(), bufferSize);
}

size_t EfiDisplay::GetHeight() const
{
	return m_device.VerticalResolution;
}

size_t EfiDisplay::GetWidth() const
{
	return m_device.HorizontalResolution;
}

Graphics::Color* EfiDisplay::GetBuffer()
{
	return (Graphics::Color*)m_address;
}