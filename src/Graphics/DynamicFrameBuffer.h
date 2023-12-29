#pragma once

#include <Graphics/Framebuffer.h>

class DynamicFrameBuffer : public Graphics::FrameBuffer
{
public:
	DynamicFrameBuffer() : m_height(), m_width(), m_buffer()
	{

	}

	~DynamicFrameBuffer()
	{
		delete[] m_buffer;
	}

	void Initialize(const size_t height, const size_t width)
	{
		m_height = height;
		m_width = width;
		m_buffer = new Graphics::Color[m_height * m_width];
	}

	virtual size_t GetHeight() const override
	{
		return m_height;
	}

	virtual size_t GetWidth() const override
	{
		return m_width;
	}

	virtual Graphics::Color* GetBuffer() override
	{
		return m_buffer;
	}

private:
	size_t m_height;
	size_t m_width;
	Graphics::Color* m_buffer;
};
