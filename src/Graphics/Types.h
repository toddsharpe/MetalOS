#pragma once

#include "core_crt/stdint.h"

namespace Graphics
{
	struct BGRRPixel
	{	
		constexpr BGRRPixel(const uint8_t b, const uint8_t g, const uint8_t r) : Blue(b), Green(g), Red(r), Reserved()
		{

		}
		
		uint8_t Blue;
		uint8_t Green;
		uint8_t Red;
		uint8_t Reserved;
	};
	static_assert(sizeof(BGRRPixel) == sizeof(uint32_t), "Pixel greater than UINT32 in size");
	typedef BGRRPixel Color;

	namespace Colors
	{
		static constexpr Color White = { 0xFF, 0xFF, 0xFF };
		static constexpr Color Blue = { 0xFF, 0x00, 0x00 };
		static constexpr Color Green = { 0x00, 0xFF, 0x00 };
		static constexpr Color Red = { 0x00, 0x00, 0xFF };
		static constexpr Color Black = { 0x00, 0x00, 0x00 };
	}
	
	struct Point2D
	{
		int32_t X;
		int32_t Y;
	};

	struct Rectangle
	{
		int32_t X;
		int32_t Y;
		uint32_t Width;
		uint32_t Height;

		bool Contains(const Point2D& p) const
		{
			if (p.X < X || p.X >= X + Width)
				return false;

			if (p.Y < Y || p.Y >= Y + Height)
				return false;

			return true;
		}
	};

	struct FrameBuffer
	{
		FrameBuffer() : Buffer(), Height(), Width()
		{

		}

		FrameBuffer(Color* const buffer, const uint32_t height, const uint32_t width) :
			Buffer(buffer),
			Height(height),
			Width(width)
		{

		}

		size_t Size() const
		{
			return static_cast<size_t>(Height) * Width * sizeof(Color);
		}

		Graphics::Rectangle GetBounds()
		{
			return { 0, 0, Width, Height };
		}

		Color* Buffer;
		uint32_t Height;
		uint32_t Width;
	};
}