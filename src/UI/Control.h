#pragma once

#include "Graphics/Types.h"

namespace UI
{
	using namespace Graphics;

	class Control
	{
	public:
		Control(const Rectangle& bounds) : Foreground(Colors::Red), Background(Colors::White), Bounds(bounds)
		{

		}

		virtual void Draw(FrameBuffer& frame) = 0;

		//The window got a left click inside Bounds; most controls ignore it
		virtual void OnClick()
		{
		}

		Color Foreground;
		Color Background;
		Rectangle Bounds; //in window coordinates
	};
}
