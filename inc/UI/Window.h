#pragma once

#include "Graphics/Color.h"
#include "Graphics/Types.h"
#include "UI/Control.h"

#include <string>
#include <vector>

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
		Window(const std::string& title, const WindowStyle& style = Default) :
			Background(Colors::White),
			Children(),
			Title(title),
			m_style(style)
		{

		}

		void Draw(FrameBuffer& frame)
		{
			//Clear frame
			frame.FillScreen(Background);
			
			//Draw border
			if (m_style.IsBordered)
			{
				frame.DrawFrameBorder(BorderColor, 3);
				frame.DrawText({ 9, 9 }, Title, BorderColor);
				frame.DrawRectangle(BorderColor, { 0, 20, frame.GetWidth(), 3 });
			}

			//Draw children
			for (const auto& child : Children)
			{
				child->Draw(frame);
			}
		}

		Color Background;
		std::vector<Control*> Children;
		std::string Title;

	private:
		WindowStyle m_style;
	};
}
