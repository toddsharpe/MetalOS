#pragma once

#include "core_crt/stdint.h"
#include "core_crt/string.h"
#include "Assert.h"

//Buffer of fixed length that is read only
struct CBuffer
{
	constexpr CBuffer() : Data(), Length()
	{
		
	}
	
	template <size_t N>
	constexpr CBuffer(const uint8_t (&data)[N]) : Data(data), Length(N)
	{
		
	}

	constexpr CBuffer(const uint8_t *const data, const size_t length) : Data(data), Length(length)
	{
		
	}

	constexpr CBuffer(const void *const data, const size_t length) : Data((uint8_t*)data), Length(length)
	{
		
	}

	bool operator==(const CBuffer& rhs) const
	{
		return this->Length == rhs.Length && memcmp(this->Data, rhs.Data, this->Length) == 0;
	}

	const uint8_t* Data;
	const size_t Length;
};

//Buffer of fixed length who's contents can be modified
struct Buffer
{
	template <size_t N>
	constexpr Buffer(uint8_t (&data)[N]) : Data(data), Length(N)
	{
		
	}

	constexpr Buffer(uint8_t *const data, const size_t length) : Data(data), Length(length)
	{
		
	}

	bool operator==(const Buffer& rhs) const
	{
		return this->Length == rhs.Length && memcmp(this->Data, rhs.Data, this->Length) == 0;
	}

	operator CBuffer() const
	{
		return {Data, Length};
	}

	const CBuffer Slice(const size_t length)
	{
		Assert(length <= Length);
		return {Data, length};
	}

	uint8_t* const Data;
	const size_t Length;
};

struct Storage
{
public:
	template <size_t N>
	constexpr Storage(uint8_t (&data)[N]) :
		Data(data),
		m_count(0),
		Capacity(N)
	{
		
	}

	size_t Count() const
	{
		return m_count;
	}

	uint8_t* const Data;
	const size_t Capacity;

private:
	size_t m_count;
};

template <size_t N>
struct StaticStorage : public Storage
{
public:
	constexpr StaticStorage() :
		Storage(m_storage)
	{

	}

private:
	uint8_t m_storage[N];
};
