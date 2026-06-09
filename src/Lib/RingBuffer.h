#pragma once

#include <cstdint>
#include <cstring>

//https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
template<size_t N>
class RingBuffer
{
public:
	constexpr RingBuffer() :
		m_storage(),
		m_read(),
		m_write()
	{
		static_assert(N > 0 && (N & (N - 1)) == 0);
	}

	bool Push(const void* const buffer, const size_t length)
	{
		if (N - Size() < length)
			return false;

		const size_t write_idx = mask(m_write);
		const size_t first = length < N - write_idx ? length : N - write_idx;
		memcpy(&m_storage[write_idx], buffer, first);
		memcpy(&m_storage[0], static_cast<const uint8_t*>(buffer) + first, length - first);
		m_write += static_cast<uint32_t>(length);
		return true;
	}

	bool Pop(void* const buffer, const size_t length, size_t& bytes_read)
	{
		if (IsEmpty())
			return false;

		bytes_read = Size() < length ? Size() : length;

		const size_t read_idx = mask(m_read);
		const size_t first = bytes_read < N - read_idx ? bytes_read : N - read_idx;
		memcpy(buffer, &m_storage[read_idx], first);
		memcpy(static_cast<uint8_t*>(buffer) + first, &m_storage[0], bytes_read - first);
		m_read += static_cast<uint32_t>(bytes_read);
		return true;
	}

	size_t Size() const
	{
		return m_write - m_read;
	}

	bool IsEmpty() const
	{
		return m_read == m_write;
	}

private:
	constexpr size_t mask(const uint32_t value) const
	{
		constexpr size_t bitmask = N - 1;
		return value & bitmask;
	}

	uint8_t m_storage[N];

	uint32_t m_read;
	uint32_t m_write;
};
