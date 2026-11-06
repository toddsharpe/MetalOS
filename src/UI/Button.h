#pragma once

#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "Graphics/Theme.h"
#include "UI/Control.h"

namespace UI
{
	using namespace Graphics;

	//What a button does decides how it looks: plain, an operator, the main action or a destructive one
	enum class ButtonStyle
	{
		Normal,
		Operator,
		Primary,
		Danger,
	};

	class Button;
	typedef void(*ButtonClick)(Button& button);

	class Button : public Control
	{
	public:
		Button(const cstr_t text, const Rectangle& bounds, const ButtonStyle style = ButtonStyle::Normal) :
			Control(bounds),
			Text(),
			Style(style),
			Click()
		{
			StrCopy(Text, text);
		}

		virtual void Draw(FrameBuffer& frame) override
		{
			const Theme::Face& face = Face();
			Draw2D::DrawFace(frame, Bounds, face);

			//Centered, and nudged down a pixel while pressed so it reads as pushed in
			const cstr_t text = Str(Text);
			const uint32_t width = Draw2D::PixelTextWidth(text.Size, Theme::TextScale);
			const int32_t x = Bounds.X + ((int32_t)Bounds.Width - (int32_t)width) / 2;
			const int32_t y = Bounds.Y + ((int32_t)Bounds.Height - Theme::Glyph) / 2 + (Pressed ? 1 : 0);
			Draw2D::DrawPixelText(frame, { x, y }, text, face.Label, Theme::TextScale);
		}

		virtual void OnClick() override
		{
			if (Click)
				Click(*this);
		}

		char Text[64];
		ButtonStyle Style;
		ButtonClick Click;

	private:
		const Theme::Face& Face() const
		{
			switch (Style)
			{
				case ButtonStyle::Operator:
					return Pressed ? Theme::OperatorSunken : (Hovered ? Theme::OperatorHover : Theme::Operator);

				case ButtonStyle::Primary:
					return Pressed ? Theme::PrimarySunken : (Hovered ? Theme::PrimaryHover : Theme::Primary);

				case ButtonStyle::Danger:
					return Pressed ? Theme::DangerSunken : (Hovered ? Theme::DangerLift : Theme::Danger);

				default:
					return Pressed ? Theme::Sunken : (Hovered ? Theme::RaisedHover : Theme::Raised);
			}
		}
	};
}
