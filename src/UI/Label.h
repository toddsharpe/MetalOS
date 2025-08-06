#pragma once

#include "Lib/String.h"
#include "UI/Control.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"

namespace UI
{
	using namespace Graphics;

	class Label : public Control
	{
	public:
		Label(const CString& text, const Rectangle& bounds) :
			Control(),
			Bounds(bounds),
			Text(text)
		{

		}

		virtual void Draw(FrameBuffer& frame) override
		{
			Draw2D::DrawRectangle(frame, Background, Bounds);
			Draw2D::DrawText(frame, { Bounds.X + 5, Bounds.Y + 5 }, Text, Foreground);
		}

		Rectangle Bounds;
		StaticString<512> Text;
	};
}
