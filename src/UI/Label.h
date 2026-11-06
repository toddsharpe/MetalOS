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
		Label(const cstr_t text, const Rectangle& bounds, const uint32_t scale = Theme::TextScale) :
			Control(bounds),
			Text(),
			Scale(scale)
		{
			StrCopy(Text, text);
		}

		virtual void Draw(FrameBuffer& frame) override
		{
			Draw2D::DrawRectangle(frame, Background, Bounds);
			Draw2D::DrawPixelText(frame, { Bounds.X + 5, Bounds.Y + 5 }, Str(Text), Foreground, Scale);
		}

		char Text[512];
		uint32_t Scale;
	};
}
