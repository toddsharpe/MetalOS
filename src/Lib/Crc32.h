#pragma once

#include <cstddef>
#include <cstdint>

//CRC-32 as GPT, zip and Ethernet use it (reflected polynomial 0xEDB88320, inverted at both ends), added up in parts
class Crc32
{
public:
	constexpr Crc32() : m_state(0xFFFFFFFF)
	{

	}

	void Add(const void* const data, const size_t length)
	{
		const uint8_t* const bytes = static_cast<const uint8_t*>(data);
		for (size_t i = 0; i < length; i++)
		{
			m_state ^= bytes[i];
			for (size_t bit = 0; bit < 8; bit++)
				m_state = (m_state >> 1) ^ (0xEDB88320 & (0 - (m_state & 1)));
		}
	}

	uint32_t Value() const
	{
		return ~m_state;
	}

	static uint32_t Of(const void* const data, const size_t length)
	{
		Crc32 crc;
		crc.Add(data, length);
		return crc.Value();
	}

private:
	uint32_t m_state;
};
