#pragma once

#include "Lib/String.h"
#include "UI/Control.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "Graphics/Theme.h"

namespace UI
{
	using namespace Graphics;

	class Label : public Control
	{
	public:
		Label(const CString& text, const Rectangle& bounds, const uint32_t scale = Theme::TextScale) :
			Control(bounds),
			Text(text),
			Scale(scale)
		{
		}

		virtual void Draw(FrameBuffer& frame) override
		{
			Draw2D::DrawRectangle(frame, Background, Bounds);
			Draw2D::DrawPixelText(frame, { Bounds.X + 5, Bounds.Y + 5 }, Text, Foreground, Scale);
		}

		StaticString<512> Text;
		uint32_t Scale;
	};
}
