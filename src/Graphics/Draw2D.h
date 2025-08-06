#pragma once

#include "Lib/Math.h"
#include "Graphics/Types.h"
#include "Graphics/Font.h"

namespace Graphics
{
	namespace Draw2D
	{
		static constexpr size_t PixelScale = 10;
		static constexpr size_t FontScale = 1;
		static Font m_font;

		static void SetPixels(Color *_Dst, const Color _Val, const size_t _Size)
		{
			for (size_t i = 0; i < _Size; i++)
			{
				*_Dst = _Val;
				_Dst++;
			}
		}

		void DrawRectangle(Graphics::FrameBuffer &frame, const Color color, const Rectangle &region)
		{
			const int32_t x0 = Max<int32_t>(0, region.X);
			const int32_t x1 = Min<int32_t>(static_cast<int32_t>(frame.Width),  region.X + static_cast<int32_t>(region.Width));
			const int32_t y0 = Max<int32_t>(0, region.Y);
			const int32_t y1 = Min<int32_t>(static_cast<int32_t>(frame.Height), region.Y + static_cast<int32_t>(region.Height));
			if (x0 >= x1 || y0 >= y1)
				return;

			const size_t visibleWidth  = static_cast<size_t>(x1 - x0);
			const size_t visibleHeight = static_cast<size_t>(y1 - y0);

			for (size_t y = 0; y < visibleHeight; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(y0) + y) * frame.Width + static_cast<size_t>(x0);
				SetPixels(dst, color, visibleWidth);
			}
		}

		void FillScreen(Graphics::FrameBuffer &frame, const Color color)
		{
			DrawRectangle(frame, color, {0, 0, frame.Width, frame.Height});
		}

		void DrawPoint(Graphics::FrameBuffer &frame, const Color color, const Point2D &point)
		{
			Color *dst = frame.Buffer + point.Y * static_cast<int32_t>(frame.Width) + point.X;
			*dst = color;
		}

		void DrawFrameBorder(Graphics::FrameBuffer &frame, const Color color, const size_t thickness)
		{
			// Top
			SetPixels(frame.Buffer, color, frame.Width * thickness);

			// Bottom
			Color *dst = frame.Buffer + (frame.Height - thickness) * frame.Width;
			SetPixels(dst, color, frame.Width * thickness);

			// Sides
			for (size_t y = thickness; y < frame.Height - thickness; y++)
			{
				Color *row = frame.Buffer + y * frame.Width;

				// Left side
				SetPixels(row, color, thickness);

				// Right side
				SetPixels(row + (frame.Width - thickness), color, thickness);
			}
		}

		void DrawBorder(Graphics::FrameBuffer &frame, const Color color, const Rectangle &rect, const uint32_t thickness)
		{
			// Top
			DrawRectangle(frame, color, {rect.X, rect.Y, rect.Width, thickness});

			// Left
			DrawRectangle(frame, color, {rect.X, rect.Y, thickness, rect.Height});

			// Right
			DrawRectangle(frame, color, {rect.X + static_cast<int32_t>(rect.Width), rect.Y, thickness, rect.Height});

			// Bottom
			DrawRectangle(frame, color, {rect.X, rect.Y + static_cast<int32_t>(rect.Height), rect.Width, thickness});
		}

		void WriteFrame(Graphics::FrameBuffer &frame, const Rectangle &rect, const void *fromFrame)
		{
			const int32_t x0 = Max<int32_t>(0, rect.X);
			const int32_t x1 = Min<int32_t>(static_cast<int32_t>(frame.Width),  rect.X + static_cast<int32_t>(rect.Width));
			const int32_t y0 = Max<int32_t>(0, rect.Y);
			const int32_t y1 = Min<int32_t>(static_cast<int32_t>(frame.Height), rect.Y + static_cast<int32_t>(rect.Height));
			if (x0 >= x1 || y0 >= y1)
				return;

			const size_t visibleWidth  = static_cast<size_t>(x1 - x0);
			const size_t visibleHeight = static_cast<size_t>(y1 - y0);
			const size_t srcOffsetX    = static_cast<size_t>(x0 - rect.X);
			const size_t srcOffsetY    = static_cast<size_t>(y0 - rect.Y);

			for (size_t y = 0; y < visibleHeight; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(y0) + y) * frame.Width + static_cast<size_t>(x0);
				const uint32_t *src = reinterpret_cast<const uint32_t*>(fromFrame) + (srcOffsetY + y) * rect.Width + srcOffsetX;
				memcpy(dst, src, visibleWidth * sizeof(Color));
			}
		}

		void WriteFrameGrayscale(Graphics::FrameBuffer &frame, const Rectangle &rect, const void *fromFrame)
		{
			const int32_t x0 = Max<int32_t>(0, rect.X);
			const int32_t x1 = Min<int32_t>(static_cast<int32_t>(frame.Width),  rect.X + static_cast<int32_t>(rect.Width));
			const int32_t y0 = Max<int32_t>(0, rect.Y);
			const int32_t y1 = Min<int32_t>(static_cast<int32_t>(frame.Height), rect.Y + static_cast<int32_t>(rect.Height));
			if (x0 >= x1 || y0 >= y1)
				return;

			const size_t visibleWidth  = static_cast<size_t>(x1 - x0);
			const size_t visibleHeight = static_cast<size_t>(y1 - y0);
			const size_t srcOffsetX    = static_cast<size_t>(x0 - rect.X);
			const size_t srcOffsetY    = static_cast<size_t>(y0 - rect.Y);

			for (size_t y = 0; y < visibleHeight; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(y0) + y) * frame.Width + static_cast<size_t>(x0);
				const Color *src = reinterpret_cast<const Color*>(fromFrame) + (srcOffsetY + y) * rect.Width + srcOffsetX;

				for (size_t x = 0; x < visibleWidth; x++)
				{
					size_t sum = src->Blue + src->Green + src->Red;
					uint8_t average = static_cast<uint8_t>(sum / 3);

					dst->Blue = average;
					dst->Green = average;
					dst->Red = average;
					dst->Reserved = src->Reserved;

					dst++;
					src++;
				}
			}
		}

		Color GetPixel(Graphics::FrameBuffer &frame, const Point2D &position)
		{
			// Assert(position.X < frame.Width);
			// Assert(position.Y < frame.Height);

			return frame.Buffer[position.Y * static_cast<int32_t>(frame.Width) + position.X];
		}

		void DrawCharacter(Graphics::FrameBuffer &frame, const Point2D position, const char c, const Color color)
		{
			const uint8_t *map = m_font.GetCharacterMap(c);
			uint8_t size = 8; // TODO: get from font

			for (size_t y = 0; y < size; y++)
			{
				// Line is reversed, obv
				char line = map[y];

				size_t x = 0;
				unsigned char mask = 0x80;
				while (mask > 0)
				{
					if ((line & mask) != 0)
					{
						Rectangle rect;
						rect.X = (position.X + static_cast<int32_t>(8 - x - 1)) * static_cast<int32_t>(FontScale);
						rect.Y = (position.Y + static_cast<int32_t>(y)) * static_cast<int32_t>(FontScale);
						rect.Width = FontScale;
						rect.Height = FontScale;
						DrawRectangle(frame, color, rect);
					}

					mask = mask >> 1;
					x++;
				}
			}
		}

		Point2D DrawText(Graphics::FrameBuffer &frame, const Point2D position, const CString &string, const Color color)
		{
			Point2D pos = position;
			for (size_t i = 0; i < string.Length; i++)
			{
				const char c = string[i];
				if (c == '\n')
				{
					pos.Y += static_cast<int32_t>(PixelScale);
					pos.X = position.X;
				}
				else if (c != '\r')
				{
					DrawCharacter(frame, pos, c, color);
					pos.X += static_cast<int32_t>(PixelScale);
				}
			}
			return pos;
		}
	}
}
