#pragma once

#include "Lib/StaticVector.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "UI/Control.h"

namespace UI
{
	using namespace Graphics;

	struct WindowStyle
	{
		bool IsBordered;
	};

	static const WindowStyle Default = { true };

	class Window
	{
	private:
		static constexpr Color BorderColor = Colors::Blue;

	public:
		Window(const CString& title, const WindowStyle& style = Default) :
			Background(Colors::White),
			Children(),
			Title(title),
			m_style(style)
		{

		}

		void Draw(FrameBuffer& frame)
		{
			//Clear frame
			Draw2D::FillScreen(frame, Background);
			
			//Draw border
			if (m_style.IsBordered)
			{
				Draw2D::DrawFrameBorder(frame, BorderColor, 3);
				Draw2D::DrawText(frame, { 9, 9 }, Title, BorderColor);
				Draw2D::DrawRectangle(frame, BorderColor, { 0, 20, frame.Width, 3 });
			}

			//Draw children
			for (const auto& child : Children)
			{
				child->Draw(frame);
			}
		}

		Color Background;
		StaticVector<Control*, 16> Children;
		StaticString<32> Title;

	private:
		WindowStyle m_style;
	};
}
