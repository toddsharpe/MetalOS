#pragma once

#include <Graphics/Types.h>
#include <UI/Control.h>

namespace UI
{
	using namespace Graphics;

	class Button;
	typedef void(*ButtonClick)(Button& button);

	class Button : public Control
	{
	public:
		Button(const std::string& text, const Rectangle& bounds) :
			Control(),
			Bounds(bounds),
			Text(text),
			Click()
		{

		}

		virtual void Draw(FrameBuffer& frame) override
		{
			frame.DrawBorder(Foreground, Bounds, 3);
			frame.DrawText({ Bounds.X + 5, Bounds.Y + 5 }, Text.c_str(), Foreground);
		}

		Rectangle Bounds;
		std::string Text;
		ButtonClick Click;
	};
}
