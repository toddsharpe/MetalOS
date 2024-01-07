#include "Assert.h"
#include <algorithm>

#include "EfiDisplay.h"

EfiDisplay::EfiDisplay(void* const address, const size_t height, const size_t width) :
	Graphics::FrameBuffer(),
	m_buffer(reinterpret_cast<Graphics::Color*>(address)),
	m_height(height),
	m_width(width)
{
	
}

size_t EfiDisplay::GetHeight() const
{
	return m_height;
}

size_t EfiDisplay::GetWidth() const
{
	return m_width;
}

Graphics::Color* EfiDisplay::GetBuffer()
{
	return m_buffer;
}

void EfiDisplay::Write(Graphics::FrameBuffer& framebuffer)
{
	const size_t bufferSize = framebuffer.GetHeight() * framebuffer.GetWidth() * sizeof(Graphics::Color);
	memcpy(m_buffer, framebuffer.GetBuffer(), bufferSize);
}
