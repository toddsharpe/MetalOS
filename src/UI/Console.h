#pragma once

#include "Lib/TextBuffer.h"
#include "UI/Control.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"

namespace UI
{
	using namespace Graphics;

	//Terminal-style text area: draws the newest lines of its scrollback that fit, so the last one is always visible
	class Console : public Control
	{
	public:
		Console(const Rectangle& bounds) :
			Control(),
			Bounds(bounds),
			Text()
		{
			Text.SetColumns(Cells(Bounds.Width));
		}

		virtual void Draw(FrameBuffer& frame) override
		{
			Draw2D::DrawRectangle(frame, Background, Bounds);

			const size_t rows = Cells(Bounds.Height);
			const size_t count = Text.LineCount();
			const size_t first = count > rows ? count - rows : 0;

			Point2D position = { Bounds.X + Padding, Bounds.Y + Padding };
			for (size_t i = first; i < count; i++)
			{
				Draw2D::DrawText(frame, position, Text.Line(i), Foreground);
				position.Y += static_cast<int32_t>(Draw2D::PixelScale);
			}
		}

		Rectangle Bounds;
		TextBuffer<256, 128> Text;

	private:
		static constexpr int32_t Padding = 5;

		//Character cells that fit across a length of pixels, inside the padding
		static size_t Cells(const uint32_t pixels)
		{
			const uint32_t inner = pixels > 2 * Padding ? pixels - 2 * Padding : 0;
			return inner / Draw2D::PixelScale;
		}
	};
}
