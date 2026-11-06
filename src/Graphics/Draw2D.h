#pragma once

#include "Lib/Math.h"
#include "Graphics/Types.h"
#include "Graphics/Font.h"
#include "Graphics/Theme.h"

namespace Graphics
{
	namespace Draw2D
	{
		static constexpr size_t PixelScale = 10;
		static constexpr size_t FontScale = 1;
		inline Font m_font;

		inline void SetPixels(Color *_Dst, const Color _Val, const size_t _Size)
		{
			for (size_t i = 0; i < _Size; i++)
			{
				*_Dst = _Val;
				_Dst++;
			}
		}

		//The part of a rectangle inside the frame, and where that part starts within the rectangle
		struct ClipRect
		{
			int32_t X;
			int32_t Y;
			size_t Width;
			size_t Height;
			size_t OffsetX;
			size_t OffsetY;
		};

		inline bool Clip(const Graphics::FrameBuffer &frame, const Rectangle &rect, ClipRect &clip)
		{
			const int32_t x0 = Max<int32_t>(0, rect.X);
			const int32_t x1 = Min<int32_t>(static_cast<int32_t>(frame.Width),  rect.X + static_cast<int32_t>(rect.Width));
			const int32_t y0 = Max<int32_t>(0, rect.Y);
			const int32_t y1 = Min<int32_t>(static_cast<int32_t>(frame.Height), rect.Y + static_cast<int32_t>(rect.Height));
			if (x0 >= x1 || y0 >= y1)
				return false;

			clip = { x0, y0, static_cast<size_t>(x1 - x0), static_cast<size_t>(y1 - y0), static_cast<size_t>(x0 - rect.X), static_cast<size_t>(y0 - rect.Y) };
			return true;
		}

		inline void DrawRectangle(const Graphics::FrameBuffer &frame, const Color color, const Rectangle &region)
		{
			ClipRect clip;
			if (!Clip(frame, region, clip))
				return;

			for (size_t y = 0; y < clip.Height; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(clip.Y) + y) * frame.Width + static_cast<size_t>(clip.X);
				SetPixels(dst, color, clip.Width);
			}
		}

		inline void FillScreen(const Graphics::FrameBuffer &frame, const Color color)
		{
			DrawRectangle(frame, color, {0, 0, frame.Width, frame.Height});
		}

		inline void DrawPoint(Graphics::FrameBuffer &frame, const Color color, const Point2D &point)
		{
			Color *dst = frame.Buffer + point.Y * static_cast<int32_t>(frame.Width) + point.X;
			*dst = color;
		}

		inline void DrawFrameBorder(Graphics::FrameBuffer &frame, const Color color, const size_t thickness)
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

		inline void DrawBorder(Graphics::FrameBuffer &frame, const Color color, const Rectangle &rect, const uint32_t thickness)
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

		inline void WriteFrame(Graphics::FrameBuffer &frame, const Rectangle &rect, const void *fromFrame)
		{
			ClipRect clip;
			if (!Clip(frame, rect, clip))
				return;

			for (size_t y = 0; y < clip.Height; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(clip.Y) + y) * frame.Width + static_cast<size_t>(clip.X);
				const uint32_t *src = reinterpret_cast<const uint32_t*>(fromFrame) + (clip.OffsetY + y) * rect.Width + clip.OffsetX;
				memcpy(dst, src, clip.Width * sizeof(Color));
			}
		}

		inline void WriteFrameGrayscale(Graphics::FrameBuffer &frame, const Rectangle &rect, const void *fromFrame)
		{
			ClipRect clip;
			if (!Clip(frame, rect, clip))
				return;

			for (size_t y = 0; y < clip.Height; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(clip.Y) + y) * frame.Width + static_cast<size_t>(clip.X);
				const Color *src = reinterpret_cast<const Color*>(fromFrame) + (clip.OffsetY + y) * rect.Width + clip.OffsetX;

				for (size_t x = 0; x < clip.Width; x++)
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

		inline Color GetPixel(Graphics::FrameBuffer &frame, const Point2D &position)
		{
			// Assert(position.X < frame.Width);
			// Assert(position.Y < frame.Height);

			return frame.Buffer[position.Y * static_cast<int32_t>(frame.Width) + position.X];
		}

		inline void DrawCharacter(Graphics::FrameBuffer &frame, const Point2D position, const char c, const Color color)
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

		inline Point2D DrawText(Graphics::FrameBuffer &frame, const Point2D position, const CString &string, const Color color)
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

		//The color num/den of the way from a to b
		inline Color Blend(const Color a, const Color b, const uint32_t num, const uint32_t den)
		{
			if (den == 0)
				return a;

			return Color(
				static_cast<uint8_t>((a.Blue * (den - num) + b.Blue * num) / den),
				static_cast<uint8_t>((a.Green * (den - num) + b.Green * num) / den),
				static_cast<uint8_t>((a.Red * (den - num) + b.Red * num) / den));
		}

		//The color darkened to keep/256 of itself
		inline Color Shade(const Color c, const uint32_t keep)
		{
			return Color(static_cast<uint8_t>((c.Blue * keep) >> 8), static_cast<uint8_t>((c.Green * keep) >> 8), static_cast<uint8_t>((c.Red * keep) >> 8));
		}

		inline void FillGradient(const Graphics::FrameBuffer &frame, const Rectangle &rect, const Color top, const Color bottom)
		{
			const uint32_t span = rect.Height > 1 ? rect.Height - 1 : 1;
			for (uint32_t y = 0; y < rect.Height; y++)
				DrawRectangle(frame, Blend(top, bottom, y, span), { rect.X, rect.Y + static_cast<int32_t>(y), rect.Width, 1 });
		}

		//A 1px line just inside each edge of the rectangle
		inline void DrawOutline(const Graphics::FrameBuffer &frame, const Rectangle &rect, const Color color)
		{
			if (rect.Width == 0 || rect.Height == 0)
				return;

			DrawRectangle(frame, color, { rect.X, rect.Y, rect.Width, 1 });
			DrawRectangle(frame, color, { rect.X, rect.Y + static_cast<int32_t>(rect.Height) - 1, rect.Width, 1 });
			DrawRectangle(frame, color, { rect.X, rect.Y, 1, rect.Height });
			DrawRectangle(frame, color, { rect.X + static_cast<int32_t>(rect.Width) - 1, rect.Y, 1, rect.Height });
		}

		//Light along the top and left edges, dark along the bottom and right, so the rectangle looks raised (or sunken, swapped)
		inline void DrawBevel(const Graphics::FrameBuffer &frame, const Rectangle &rect, const Color light, const Color dark)
		{
			if (rect.Width == 0 || rect.Height == 0)
				return;

			DrawRectangle(frame, light, { rect.X, rect.Y, rect.Width, 1 });
			DrawRectangle(frame, light, { rect.X, rect.Y, 1, rect.Height });
			DrawRectangle(frame, dark, { rect.X, rect.Y + static_cast<int32_t>(rect.Height) - 1, rect.Width, 1 });
			DrawRectangle(frame, dark, { rect.X + static_cast<int32_t>(rect.Width) - 1, rect.Y, 1, rect.Height });
		}

		//Darkens what is already drawn in the rectangle to keep/256 (shadows, dimmed areas)
		inline void Darken(const Graphics::FrameBuffer &frame, const Rectangle &rect, const uint32_t keep)
		{
			ClipRect clip;
			if (!Clip(frame, rect, clip))
				return;

			for (size_t y = 0; y < clip.Height; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(clip.Y) + y) * frame.Width + static_cast<size_t>(clip.X);
				for (size_t x = 0; x < clip.Width; x++)
					dst[x] = Shade(dst[x], keep);
			}
		}

		constexpr uint32_t PixelTextWidth(const size_t length, const uint32_t scale)
		{
			return static_cast<uint32_t>(length) * 8 * scale;
		}

		//The system font at a whole-number scale on an 8*scale advance (the glyphs carry their own spacing); returns where the next character goes
		inline Point2D DrawPixelText(const Graphics::FrameBuffer &frame, const Point2D position, const CString &text, const Color color, const uint32_t scale)
		{
			Point2D pos = position;
			for (size_t i = 0; i < text.Length; i++)
			{
				const uint8_t *map = m_font.GetCharacterMap(text[i]);
				for (uint32_t row = 0; row < 8; row++)
				{
					//Bit n is column n, and each run of set bits is drawn as one rectangle
					const uint8_t bits = map[row];
					uint32_t col = 0;
					while (col < 8)
					{
						if ((bits & (1u << col)) == 0)
						{
							col++;
							continue;
						}

						const uint32_t start = col;
						while (col < 8 && (bits & (1u << col)) != 0)
							col++;
						DrawRectangle(frame, color, { pos.X + static_cast<int32_t>(start * scale), pos.Y + static_cast<int32_t>(row * scale), (col - start) * scale, scale });
					}
				}
				pos.X += static_cast<int32_t>(8 * scale);
			}
			return pos;
		}

		//Pixel art at a whole-number scale, its colors kept at keep/256 (256 draws them as they are)
		inline void DrawGlyph(const Graphics::FrameBuffer &frame, const Point2D position, const PixelGlyph &glyph, const uint32_t scale, const uint32_t keep = 256)
		{
			for (uint32_t y = 0; y < glyph.Height; y++)
			{
				const char *row = glyph.Rows[y];
				for (uint32_t x = 0; x < glyph.Width && row[x] != '\0'; x++)
				{
					if (row[x] < 'a' || row[x] > 'c')
						continue;

					const Color color = Shade(glyph.Palette[row[x] - 'a'], keep);
					DrawRectangle(frame, color, { position.X + static_cast<int32_t>(x * scale), position.Y + static_cast<int32_t>(y * scale), scale, scale });
				}
			}
		}

		//Like WriteFrame, with every pixel darkened to keep/256
		inline void WriteFrameDimmed(const Graphics::FrameBuffer &frame, const Rectangle &rect, const void *fromFrame, const uint32_t keep)
		{
			ClipRect clip;
			if (!Clip(frame, rect, clip))
				return;

			for (size_t y = 0; y < clip.Height; y++)
			{
				Color *dst = frame.Buffer + (static_cast<size_t>(clip.Y) + y) * frame.Width + static_cast<size_t>(clip.X);
				const Color *src = reinterpret_cast<const Color*>(fromFrame) + (clip.OffsetY + y) * rect.Width + clip.OffsetX;
				for (size_t x = 0; x < clip.Width; x++)
					dst[x] = Shade(src[x], keep);
			}
		}

		//A srcWidth x srcHeight image stretched to fill the rectangle, nearest pixel
		inline void WriteFrameScaled(const Graphics::FrameBuffer &frame, const Rectangle &rect, const void *fromFrame, const uint32_t srcWidth, const uint32_t srcHeight)
		{
			ClipRect clip;
			if (rect.Width == 0 || rect.Height == 0 || !Clip(frame, rect, clip))
				return;

			const Color *src = reinterpret_cast<const Color*>(fromFrame);
			const uint64_t stepX = (static_cast<uint64_t>(srcWidth) << 16) / rect.Width;
			const uint64_t stepY = (static_cast<uint64_t>(srcHeight) << 16) / rect.Height;
			for (size_t y = 0; y < clip.Height; y++)
			{
				const Color *srcRow = src + static_cast<size_t>(((clip.OffsetY + y) * stepY) >> 16) * srcWidth;
				Color *dst = frame.Buffer + (static_cast<size_t>(clip.Y) + y) * frame.Width + static_cast<size_t>(clip.X);
				uint64_t sx = clip.OffsetX * stepX;
				for (size_t x = 0; x < clip.Width; x++, sx += stepX)
					dst[x] = srcRow[sx >> 16];
			}
		}

		//A raised (or sunken) control: a dark outline around a gradient face with a 1px bevel
		inline void DrawFace(const Graphics::FrameBuffer &frame, const Rectangle &rect, const Theme::Face &face)
		{
			if (rect.Width < 3 || rect.Height < 3)
				return;

			DrawOutline(frame, rect, Theme::Rgb(0x08090A));
			const Rectangle inner = { rect.X + 1, rect.Y + 1, rect.Width - 2, rect.Height - 2 };
			FillGradient(frame, inner, face.Top, face.Bottom);
			DrawBevel(frame, inner, face.Light, face.Dark);
		}
	}
}
