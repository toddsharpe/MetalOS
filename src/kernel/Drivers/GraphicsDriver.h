#pragma once

#include "Graphics/FrameBuffer.h"

class GraphicsDriver
{
public:
	virtual bool Write(Graphics::FrameBuffer& framebuffer) = 0;
};
