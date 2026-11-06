#pragma once

#include "Lib/TextBuffer.h"
#include "UI/Control.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "Graphics/Theme.h"
#include "user/MetalOS.h"

namespace UI
{
	using namespace Graphics;

	//What a run of console text is, which picks its color; written into the TextBuffer with SetAttribute
	enum ConsoleAttribute : uint8_t
	{
		Normal,
		Prompt,
		Punctuation,
		Error,
		Dim,
		Bright,
	};

	//Terminal-style text area: draws the newest lines of its scrollback that fit (or older ones when scrolled back), so the last one is normally visible
	class Console : public Control
	{
	public:
		Console(const Rectangle& bounds) :
			Control(bounds),
			Text(),
			Scroll(),
			ShowCursor(true)
		{
			Background = Theme::IronDeep;
			Text.SetColumns(Cells(Bounds.Width));
		}

		//New text wraps at the new width; lines already written keep theirs
		void SetBounds(const Rectangle& bounds)
		{
			Bounds = bounds;
			Text.SetColumns(Cells(Bounds.Width));
		}

		size_t Rows() const
		{
			return Bounds.Height > 2 * PaddingY ? (Bounds.Height - 2 * PaddingY) / LineHeight : 0;
		}

		//Moves the view back (positive) or toward the newest lines (negative)
		void ScrollBy(const int64_t lines)
		{
			const int64_t scrolled = (int64_t)Scroll + lines;
			Scroll = scrolled < 0 ? 0 : (size_t)scrolled;
			ClampScroll();
		}

		virtual void Draw(FrameBuffer& frame) override
		{
			Draw2D::DrawRectangle(frame, Background, Bounds);
			ClampScroll();

			const size_t rows = Rows();
			const size_t count = Text.LineCount();
			const size_t end = count - Scroll;
			const size_t first = end > rows ? end - rows : 0;

			Point2D position = { Bounds.X + PaddingX, Bounds.Y + PaddingY };
			for (size_t i = first; i < end; i++)
			{
				DrawLine(frame, position, i);
				position.Y += LineHeight;
			}

			//A blinking block at the end of the newest line, when it's in view
			if (ShowCursor && Scroll == 0 && (GetTickCount() / 530) % 2 == 0 && end > first)
			{
				const int32_t x = Bounds.X + PaddingX + (int32_t)(Text.CurrentLength() * Theme::Glyph);
				const int32_t y = position.Y - LineHeight;
				Draw2D::DrawRectangle(frame, Theme::Ember, { x, y, (uint32_t)Theme::Glyph, (uint32_t)Theme::Glyph });
			}

			DrawScrollbar(frame, rows, count);
		}

		TextBuffer<256, 160> Text;
		size_t Scroll; //lines back from the newest
		bool ShowCursor;

	private:
		static constexpr int32_t PaddingX = 14;
		static constexpr int32_t PaddingY = 12;
		static constexpr int32_t ScrollbarSpace = 34;
		static constexpr int32_t LineHeight = 20;

		//Character cells that fit across a width, inside the padding and clear of the scrollbar
		static size_t Cells(const uint32_t pixels)
		{
			const uint32_t used = PaddingX + ScrollbarSpace;
			return pixels > used ? (pixels - used) / Theme::Glyph : 1;
		}

		static Color AttributeColor(const uint8_t attribute)
		{
			switch (attribute)
			{
				case Prompt:
					return Theme::EmberText;
				case Punctuation:
					return Theme::Steel;
				case Error:
					return Theme::ErrorText;
				case Dim:
					return Theme::Muted;
				case Bright:
					return Theme::Bone;
				default:
					return Theme::Text;
			}
		}

		void ClampScroll()
		{
			const size_t rows = Rows();
			const size_t count = Text.LineCount();
			const size_t most = count > rows ? count - rows : 0;
			if (Scroll > most)
				Scroll = most;
		}

		//One line, as runs of characters that share an attribute
		void DrawLine(FrameBuffer& frame, const Point2D position, const size_t index)
		{
			const cstr_t line = Text.Line(index);
			const uint8_t* const attributes = Text.Attributes(index);
			size_t start = 0;
			while (start < line.Size)
			{
				size_t end = start + 1;
				while (end < line.Size && attributes[end] == attributes[start])
					end++;

				const Point2D at = { position.X + (int32_t)(start * Theme::Glyph), position.Y };
				Draw2D::DrawPixelText(frame, at, line.Slice(start, end - start), AttributeColor(attributes[start]), Theme::TextScale);
				start = end;
			}
		}

		//A thin track down the right edge, with a thumb for the part in view
		void DrawScrollbar(FrameBuffer& frame, const size_t rows, const size_t count)
		{
			if (Bounds.Height <= 2 * PaddingY)
				return;

			const Rectangle track = { Bounds.X + (int32_t)Bounds.Width - 16, Bounds.Y + PaddingY, 8, Bounds.Height - 2 * PaddingY };
			Draw2D::DrawRectangle(frame, Theme::Rgb(0x16171A), track);

			uint32_t thumbHeight = track.Height;
			int32_t thumbY = track.Y;
			if (count > rows && rows != 0)
			{
				const size_t most = count - rows;
				thumbHeight = (uint32_t)((uint64_t)track.Height * rows / count);
				if (thumbHeight < 24)
					thumbHeight = 24;
				thumbY = track.Y + (int32_t)((uint64_t)(track.Height - thumbHeight) * (most - Scroll) / most);
			}

			const Rectangle thumb = { track.X, thumbY, track.Width, thumbHeight };
			Draw2D::DrawRectangle(frame, Theme::Rgb(0x3A3D44), thumb);
			Draw2D::DrawBevel(frame, thumb, Theme::Rgb(0x50545C), Theme::Rgb(0x3A3D44));
		}
	};
}
