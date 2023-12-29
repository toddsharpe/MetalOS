#pragma once

#include <Graphics/Framebuffer.h>

class GraphicsDriver
{
public:
	virtual bool Write(Graphics::FrameBuffer& framebuffer) = 0;
};
