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

	template <typename T>
	T Read()
	{
		constexpr size_t size = sizeof(T);
		Assert(m_offset <= m_length - size);

		T temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		return ByteSwap<T>(temp);
	}

	uint8_t ReadU8()
	{
		constexpr size_t size = sizeof(uint8_t);
		Assert(m_offset <= m_length - size);

		uint8_t ret = m_buffer[m_offset];
		m_offset++;

		return ret;
	}

	int8_t ReadI8()
	{
		constexpr size_t size = sizeof(int8_t);
		Assert(m_offset <= m_length - size);

		int8_t ret = m_buffer[m_offset];
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

	int16_t ReadI16()
	{
		constexpr size_t size = sizeof(int16_t);
		Assert(m_offset <= m_length - size);

		int16_t temp = {};
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

	int32_t ReadI32()
	{
		constexpr size_t size = sizeof(int32_t);
		Assert(m_offset <= m_length - size);

		int32_t temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		temp = ByteSwap(temp);

		return temp;
	}

	uint64_t ReadU64()
	{
		constexpr size_t size = sizeof(uint64_t);
		Assert(m_offset <= m_length - size);

		uint64_t temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		temp = ByteSwap(temp);

		return temp;
	}

	int64_t ReadI64()
	{
		constexpr size_t size = sizeof(int64_t);
		Assert(m_offset <= m_length - size);

		int64_t temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		temp = ByteSwap(temp);

		return temp;
	}

	float ReadF32()
	{
		constexpr size_t size = sizeof(float);
		AssertOp(m_offset, <=, m_length - size);

		float temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		temp = ByteSwap<float>(temp);

		return temp;
	}

	double ReadF64()
	{
		constexpr size_t size = sizeof(double);
		AssertOp(m_offset, <=, m_length - size);

		double temp = {};
		memcpy(&temp, &m_buffer[m_offset], size);
		m_offset += size;
		temp = ByteSwap<double>(temp);

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

	template <size_t N>
	void ReadBytes(uint8_t (&dest)[N])
	{
		ReadBytes(dest, N);
	}

	void ReadBytes(void* const to, const size_t bytes)
	{
		Assert(m_offset <= m_length - bytes);

		memcpy(to, &m_buffer[m_offset], bytes);
		m_offset += bytes;
	}

	void ReadString(char* const to, const size_t maxSize)
	{
		for (size_t i = 0; i < maxSize; i++)
		{
			to[i] = static_cast<char>(m_buffer[m_offset]);
			m_offset++;

			if (to[i] == '\0')
				return;

			if (m_offset == m_length)
				return;
		}
	}

	CString ReadCStr()
	{
		const char* head = reinterpret_cast<const char*>(Peek());
		const size_t len = strlen(head);
		m_offset += len + 1;
		return { head, len };
	}

	void SkipStr(const size_t maxSize)
	{
		for (size_t i = 0; i < maxSize; i++)
		{
			const char c = static_cast<char>(m_buffer[m_offset]);
			m_offset++;

			if (c == '\0')
				return;

			if (m_offset == m_length)
				return;
		}
	}

	void Skip(const size_t size)
	{
		Assert(m_offset <= m_length - size);
		m_offset += size;
	}

	const uint8_t* Peek() const
	{
		return &m_buffer[m_offset];
	}

	size_t BytesRead() const
	{
		return m_offset;
	}

	size_t Remaining() const
	{
		return m_length - m_offset;
	}

	bool HasData() const
	{
		return Remaining() > 0;
	}

private:
	const uint8_t* const m_buffer;
	const size_t m_length;
	size_t m_offset;
};
