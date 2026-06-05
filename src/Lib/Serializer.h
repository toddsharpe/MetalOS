#pragma once

#include "Assert.h"
#include "Lib/ByteSwap.h"

class Serializer
{
public:
	//TODO(tsharpe): Rewrite to take a buffer and track bytes in count?

	template <size_t N>
	Serializer(uint8_t (&buffer)[N]) : m_buffer(buffer), m_length(N), m_offset()
	{

	}

	Serializer(uint8_t* const buffer, const size_t length) : m_buffer(buffer), m_length(length), m_offset()
	{

	}

	template <typename T>
	void Write(const T value)
	{
		constexpr size_t size = sizeof(T);
		AssertOp(m_offset, <, m_length - size);

		const T swapped = ByteSwap<T>(value);
		memcpy(&m_buffer[m_offset], &swapped, size);
		m_offset += size;
	}

	template <size_t N>
	void Write(const uint8_t (&from)[N])
	{
		Assert(m_offset <= m_length - N);
		memcpy(&m_buffer[m_offset], from, N);

		//Endian swap
		for (size_t i = 0; i < N / 2; i++)
		{
			uint8_t temp = m_buffer[i + m_offset];
			m_buffer[i + m_offset] = m_buffer[m_offset + N - i - 1];
			m_buffer[m_offset + N - i - 1] = temp;
		}

		m_offset += N;
	}

	void Write(const char* const str)
	{
		WriteBytes(str, strlen(str));
		Write('\0');
	}

	template <size_t N>
	void WriteBytes(const uint8_t (&from)[N])
	{
		WriteBytes(from, N);
	}

	//Does no endian swap
	void WriteBytes(const void* const from, const size_t bytes)
	{
		Assert(m_offset <= m_length - bytes);
		memcpy(&m_buffer[m_offset], from, bytes);

		m_offset += bytes;
	}

	size_t Bytes() const
	{
		return m_offset;
	}

	void Reset()
	{
		m_offset = 0;
	}

	void Skip(const size_t skip)
	{
		const size_t remaining = m_length - m_offset;
		Assert(remaining >= skip);
		m_offset += skip;
	}

	void PadTo(const size_t size)
	{
		if (m_offset > size)
			return;
		Assert(size <= m_length);

		const size_t diff = size - m_offset;
		m_offset += diff;
	}

private:
	uint8_t* const m_buffer;
	const size_t m_length;
	size_t m_offset;
};
