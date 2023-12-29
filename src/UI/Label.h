#pragma once

#include <UI/Control.h>
#include <Graphics/Types.h>
#include <Graphics/Framebuffer.h>

#include <string>

namespace UI
{
	using namespace Graphics;

	class Label : public Control
	{
	public:
		Label(const std::string& text, const Rectangle& bounds) :
			Control(),
			Bounds(bounds),
			Text(text)
		{

		}

		virtual void Draw(FrameBuffer& frame) override
		{
			frame.DrawRectangle(Background, Bounds);
			frame.DrawText({ Bounds.X + 5, Bounds.Y + 5 }, Text.c_str(), Foreground);
		}

		Rectangle Bounds;
		std::string Text;
	};
}
