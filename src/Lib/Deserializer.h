#pragma once

#include "Assert.h"
#include "Lib/ByteSwap.h"

#include <cstdint>
#include <cstddef>
#include <cstring>

class Deserializer
{
public:
	Deserializer(const uint8_t* const buffer, const size_t length) :
		m_buffer(buffer),
		m_length(length),
		m_offset()
	{

	}

	uint8_t ReadU8()
	{
		constexpr size_t size = sizeof(uint8_t);
		Assert(m_offset <= m_length - size);

		uint8_t ret = m_buffer[m_offset];
		m_offset++;

		return ret;
	}

	uint16_t ReadU16()
	{
		constexpr size_t size = sizeof(uint16_t);
		Assert(m_offset <= m_length - size);

		uint16_t temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		temp = ByteSwap(temp);

		return temp;
	}

	uint32_t ReadU32()
	{
		constexpr size_t size = sizeof(uint32_t);
		Assert(m_offset <= m_length - size);

		uint32_t temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		temp = ByteSwap(temp);

		return temp;
	}

	template <size_t N>
	void Read(uint8_t (&dest)[N])
	{
		Assert(m_offset <= m_length - N);

		memcpy(dest, &m_buffer[m_offset], N);
		m_offset += N;

		//Endian swap
		for (size_t i = 0; i < N / 2; i++)
		{
			uint8_t temp = dest[i];
			dest[i] = dest[N - i - 1];
			dest[N - i - 1] = temp;
		}
	}

	void ReadBytes(void* const to, const size_t bytes)
	{
		Assert(m_offset <= m_length - bytes);

		memcpy(to, &m_buffer[m_offset], bytes);
		m_offset += bytes;
	}

	const uint8_t* Peek() const
	{
		return &m_buffer[m_offset];
	}

	size_t Remaining() const
	{
		return m_length - m_offset;
	}

private:
	const uint8_t* const m_buffer;
	const size_t m_length;
	size_t m_offset;
};
