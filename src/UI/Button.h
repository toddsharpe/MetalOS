#pragma once

#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "UI/Control.h"

namespace UI
{
	using namespace Graphics;

	class Button;
	typedef void(*ButtonClick)(Button& button);

	class Button : public Control
	{
	public:
		Button(const CString& text, const Rectangle& bounds) :
			Control(bounds),
			Text(text),
			Click()
		{

		}

		virtual void Draw(FrameBuffer& frame) override
		{
			Draw2D::DrawBorder(frame, Foreground, Bounds, 3);
			Draw2D::DrawText(frame, { Bounds.X + 5, Bounds.Y + 5 }, Text, Foreground);
		}

		virtual void OnClick() override
		{
			if (Click)
				Click(*this);
		}

		StaticString<64> Text;
		ButtonClick Click;
	};
}
