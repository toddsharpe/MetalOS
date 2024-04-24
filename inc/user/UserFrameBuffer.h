#pragma once

#include <Graphics/FrameBuffer.h>
#include <user/MetalOS.h>
#include <user/Assert.h>

class UserFrameBuffer : public Graphics::FrameBuffer
{
public:
	UserFrameBuffer(const Graphics::Rectangle& rect) :
		m_rect(rect),
		m_buffer()
	{

	}

	void Initialize()
	{
		m_buffer.Length = m_rect.Height * m_rect.Width * sizeof(Graphics::Color);
		m_buffer.Data = VirtualAlloc(nullptr, m_buffer.Length);
		Assert(m_buffer.Data);
	}

	virtual size_t GetHeight() const override
	{
		return m_rect.Height;
	}


	virtual size_t GetWidth() const override
	{
		return m_rect.Width;
	}

	virtual Graphics::Color* GetBuffer() override
	{
		return (Graphics::Color*)m_buffer.Data;
	}

private:
	const Graphics::Rectangle& m_rect;
	Buffer m_buffer;
};
