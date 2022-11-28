#pragma once

#include "Graphics/Color.h"
#include "Graphics/FrameBuffer.h"

namespace UI
{
	using namespace Graphics;

	class Control
	{
	public:
		Control() : Foreground(Colors::Red), Background(Colors::White)
		{

		}

		virtual void Draw(FrameBuffer& frame) = 0;

		Color Foreground;
		Color Background;
	};
}
