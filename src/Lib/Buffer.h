#pragma once

#include "core_crt/stdint.h"
#include "core_crt/string.h"
#include "Assert.h"

//Buffer of fixed length that is read only
struct CBuffer
{
	constexpr CBuffer() : Data(), Size()
	{
		
	}
	
	template <size_t N>
	constexpr CBuffer(const uint8_t (&data)[N]) : Data(data), Size(N)
	{
		
	}

	constexpr CBuffer(const uint8_t *const data, const size_t size) : Data(data), Size(size)
	{
		
	}

	constexpr CBuffer(const void *const data, const size_t size) : Data((uint8_t*)data), Size(size)
	{
		
	}

	bool operator==(const CBuffer& rhs) const
	{
		return this->Size == rhs.Size && memcmp(this->Data, rhs.Data, this->Size) == 0;
	}

	const uint8_t* Data;
	const size_t Size;
};

struct Buffer
{
public:
	constexpr Buffer() : Data(), Count(), Capacity()
	{

	}

	template <size_t N>
	constexpr Buffer(uint8_t (&data)[N]) :
		Data(data),
		Count(0),
		Capacity(N)
	{
		
	}

	constexpr Buffer(uint8_t *const data, const size_t length) : Data(data), Count(), Capacity(length)
	{
		
	}

	bool IsValid() const
	{
		return Data != nullptr;
	}

	CBuffer Ref() const
	{
		return { Data, Count};
	}

	uint8_t* Data;
	size_t Count;
	size_t Capacity;
};

template <size_t N>
struct StaticBuffer : public Buffer
{
public:
	constexpr StaticBuffer() :
		Buffer(m_storage),
		m_storage()
	{

	}

private:
	uint8_t m_storage[N];
};

inline void Copy(Buffer& to, const CBuffer& from)
{
	Assert(to.Capacity >= from.Size);
	memcpy(to.Data, from.Data, to.Count);
	to.Count = from.Size;
}

inline void Copy(Buffer& to, const void* from, const size_t size)
{
	Assert(to.Capacity >= size);
	memcpy(to.Data, from, size);
	to.Count = size;
}

inline void Copy(void* const to, const size_t to_size, const CBuffer&& from)
{
	Assert(to_size >= from.Size);
	memcpy(to, from.Data, from.Size);
}
