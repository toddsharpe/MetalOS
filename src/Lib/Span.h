#pragma once

#include <cstdint>
#include <cstring>
#include "Assert.h"

template <typename T>
struct IsCharType
{
	static constexpr bool Value = false;
};

template <>
struct IsCharType<char>
{
	static constexpr bool Value = true;
};

template <>
struct IsCharType<const char>
{
	static constexpr bool Value = true;
};

//Size contiguous T the span doesn't own; a Span<const char> is not null-terminated
template <typename T>
struct Span
{
	constexpr Span() : Data(), Size()
	{

	}

	constexpr Span(T* const data, const size_t size) : Data(data), Size(size)
	{

	}

	//The whole array; text goes through Str, which stops at the terminator
	template <size_t N>
	constexpr Span(T (&data)[N]) : Data(data), Size(N)
	{
		static_assert(!IsCharType<T>::Value, "View text with Str() or CStr()");
	}

	//Span<T> to Span<const T>
	template <typename U>
	constexpr Span(const Span<U>& other) : Data(other.Data), Size(other.Size)
	{

	}

	T* begin() const
	{
		return Data;
	}

	T* end() const
	{
		return Data + Size;
	}

	T& operator[](const size_t index) const
	{
		Assert(index < Size);
		return Data[index];
	}

	explicit operator bool() const
	{
		return Size != 0;
	}

	Span Slice(const size_t offset, const size_t count) const
	{
		Assert(offset <= Size && count <= Size - offset);
		return { Data + offset, count };
	}

	bool operator==(const Span<const T>& rhs) const
	{
		return Size == rhs.Size && (Size == 0 || memcmp(Data, rhs.Data, Size * sizeof(T)) == 0);
	}

	bool operator!=(const Span<const T>& rhs) const
	{
		return !(*this == rhs);
	}

	T* Data;
	size_t Size;
};

//Text and bytes; the c types are read-only views, and text is never null-terminated
using str_t = Span<char>;
using cstr_t = Span<const char>;
using buffer_t = Span<uint8_t>;
using cbuffer_t = Span<const uint8_t>;

//The bytes of any object or region
inline cbuffer_t Bytes(const void* const data, const size_t size)
{
	return { static_cast<const uint8_t*>(data), size };
}

inline buffer_t Bytes(void* const data, const size_t size)
{
	return { static_cast<uint8_t*>(data), size };
}
