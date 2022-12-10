#pragma once

#include "Graphics/Font.h"
#include "Graphics/Color.h"
#include "Graphics/Types.h"
#include "Graphics/Cursor.h"

#include <cstdarg>
#include <string>

namespace Graphics
{
	class FrameBuffer
	{
	public:
		virtual size_t GetHeight() const = 0;
		virtual size_t GetWidth() const = 0;
		virtual Color* GetBuffer() = 0;

		FrameBuffer() : m_font(), m_cursor()
		{
			
		}

		void FillScreen(const Color color)
		{
			this->DrawRectangle(color, {0, 0, GetWidth(), GetHeight()});
		}

		void DrawRectangle(const Color color, const Rectangle& region)
		{
			for (size_t y = 0; y < region.Height; y++)
			{
				if (y + region.Y >= GetHeight())
					break;

				//Write row
				Color* dst = GetBuffer() + (region.Y + y) * GetWidth() + region.X;
				const size_t count = std::min(region.Width, GetWidth() - region.X);
				SetPixels(dst, color, count);
			}
		}

		void DrawPoint(const Color color, const Point2D& point)
		{
			Color* dst = GetBuffer() + (point.Y) * GetWidth() + point.X;
			*dst = color;
		}

		void DrawFrameBorder(const Color color, const size_t thickness)
		{
			//Top
			SetPixels(GetBuffer(), color, GetWidth() * thickness);

			//Bottom
			Color* dst = (Color*)GetBuffer() + (GetHeight() - thickness) * GetWidth();
			SetPixels(dst, color, GetWidth() * thickness);

			//Sides
			for (size_t y = thickness; y < GetHeight() - thickness; y++)
			{
				Color* row = (Color*)GetBuffer() + y * GetWidth();

				//Left side
				SetPixels(row, color, thickness);

				//Right side
				SetPixels(row + (GetWidth() - thickness), color, thickness);
			}
		}

		void DrawBorder(const Color color, const Rectangle& rect, const size_t thickness)
		{
			//Top
			DrawRectangle(color, { rect.X, rect.Y, rect.Width, thickness });

			//Left
			DrawRectangle(color, { rect.X, rect.Y, thickness, rect.Height });

			//Right
			DrawRectangle(color, { rect.X + rect.Width, rect.Y, thickness, rect.Height });

			//Bottom
			DrawRectangle(color, { rect.X, rect.Y + rect.Height, rect.Width, thickness });
		}

		void WriteFrame(const Rectangle& rect, const void* frame)
		{
			for (size_t y = 0; y < rect.Height; y++)
			{
				if (y + rect.Y >= GetHeight())
					break;
				
				//Write row
				Color* dst = GetBuffer() + (rect.Y + y) * GetWidth() + rect.X;
				const uint32_t* src = (uint32_t*)frame + y * rect.Width;
				size_t count = std::min(rect.Width, GetWidth() - rect.X);
				memcpy(dst, src, count * sizeof(Color));
			}
		}

		void WriteFrameGrayscale(const Rectangle& rect, const void* frame)
		{
			for (size_t y = 0; y < rect.Height; y++)
			{
				if (y + rect.Y >= GetHeight())
					break;

				//Write row
				Color* dst = GetBuffer() + (rect.Y + y) * GetWidth() + rect.X;
				const Color* src = (Color*)frame + y * rect.Width;
				size_t count = std::min(rect.Width, GetWidth() - rect.X);

				for (size_t x = 0; x < count; x++)
				{
					size_t sum = src->Blue + src->Green + src->Red;
					uint8_t average = (uint8_t)(sum / 3);
					
					dst->Blue = average;
					dst->Green = average;
					dst->Red = average;
					dst->Reserved = src->Reserved;

					dst++;
					src++;
				}
			}
		}

		Point2D DrawText(const Point2D position, const std::string& text, const Color color)
		{
			Point2D pos = position;
			for (const char c : text)
			{
				if (c == '\n')
				{
					pos.Y += PixelScale;
					pos.X = position.X;
				}
				else if (c != '\r')
				{
					DrawCharacter(pos, c, color);
					pos.X += PixelScale;
				}
			}
			return pos;
		}

		void DrawPrintf(Point2D position, const Color color, const char* format, ...)
		{
			char buffer[255];

			va_list args;
			va_start(args, format);
			int retval = vsprintf(buffer, format, args);
			buffer[retval] = '\0';
			va_end(args);

			this->DrawText(position, std::string(buffer), color);
		}

		Color GetPixel(const Point2D& position)
		{
			//Assert(position.X < GetWidth());
			//Assert(position.Y < GetHeight());

			return GetBuffer()[position.Y * GetWidth() + position.X];
		}

		size_t Size() const
		{
			return GetHeight() * GetWidth() * sizeof(Color);
		}

		Graphics::Rectangle GetBounds()
		{
			return { 0, 0, GetWidth(), GetHeight() };
		}

		void DrawCursor(const Point2D position, const Color color)
		{
			const uint16_t* map = m_cursor.GetMap();
			size_t size = 16;

			for (size_t y = 0; y < size; y++)
			{
				const uint16_t line = map[y];
				for (size_t x = 0; x < size; x++)
				{
					const uint16_t mask = (1 << (16 - x - 1));
					if (line & mask)
					{
						this->DrawPoint(color, { position.X + x, position.Y + y });
					}
				}
			}
		}

	private:
		static constexpr size_t PixelScale = 10;
		static constexpr size_t FontScale = 1;

		static void SetPixels(Color* _Dst, const Color _Val, const size_t _Size)
		{
			for (size_t i = 0; i < _Size; i++)
			{
				*_Dst = (Color)_Val;
				_Dst++;
			}
		}

		void DrawCharacter(const Point2D position, const char c, const Color color)
		{
			const uint8_t* map = m_font.GetCharacterMap(c);
			uint8_t size = 8; // TODO: get from font

			for (size_t y = 0; y < size; y++)
			{
				//Line is reversed, obv
				char line = map[y];

				size_t x = 0;
				unsigned char mask = 0x80;
				while (mask > 0)
				{
					if ((line & mask) != 0)
					{
						Rectangle rect;
						rect.X = (position.X + (8 - x - 1)) * FontScale;
						rect.Y = (position.Y + y) * FontScale;
						rect.Width = FontScale;
						rect.Height = FontScale;
						DrawRectangle(color, rect);
					}

					mask = mask >> 1;
					x++;
				}
			}
		}

		const Font m_font;
		const Cursor m_cursor;
	};
}
