#pragma once

#include <cstdint>

namespace Graphics
{
	struct BGRRPixel
	{	
		union
		{
			struct
			{
				uint32_t Blue : 8;
				uint32_t Green : 8;
				uint32_t Red : 8;
				uint32_t Reserved : 8;
			};
			uint32_t AsUint32;
		};
	};
	static_assert(sizeof(struct BGRRPixel) == sizeof(uint32_t), "Pixel greater than UINT32 in size");
	typedef BGRRPixel Color;

	namespace Colors
	{
		static constexpr Color White = { 0xFF, 0xFF, 0xFF, 0x00 };
		static constexpr Color Blue = { 0xFF, 0x00, 0x00, 0x00 };
		static constexpr Color Green = { 0x00, 0xFF, 0x00, 0x00 };
		static constexpr Color Red = { 0x00, 0x00, 0xFF, 0x00 };
		static constexpr Color Black = { 0x00, 0x00, 0x00, 0x00 };
	}
}
