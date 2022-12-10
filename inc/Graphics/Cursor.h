#pragma once

#include <cstdint>

#include <Graphics/FrameBuffer.h>

// Constant: font8x8_basic
// Contains an 8x8 font map for unicode points U+0000 - U+007F (basic latin)

namespace Graphics
{
	class Cursor
	{
	public:
		const uint16_t* GetMap() const
		{
			return (this->cursor8x8_basic);
		}

	private:
		static constexpr uint16_t cursor8x8_basic[16] =
		{
			0b0011110000000000,
			0b0011111000000000,
			0b0011111100000000,
			0b0011111110000000,
			0b0011111111000000,
			0b0011111111100000,
			0b0011111111110000,
			0b0011111111111000,
			0b0011111111111100,
			0b0011111111111100,
			0b0011111111000000,
			0b0011110111100000,
			0b0011100111100000,
			0b0011000011110000,
			0b0000000011110000,
			0b0000000001100000
		};
	};
}
