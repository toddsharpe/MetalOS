#pragma once

//Convenient defines for all targets

constexpr size_t PageShift = 12;
constexpr size_t PageSize = (1 << PageShift);
constexpr size_t PageMask = 0xFFF;

inline constexpr size_t DivRoundUp(const size_t x, const size_t y)
{
    return (x + y - 1) / y;
}

inline constexpr size_t SizeToPages(const size_t bytes)
{
    return DivRoundUp(bytes, PageSize);
}

constexpr size_t ByteAlign(const size_t size, const size_t alignment)
{
	const size_t mask = alignment - 1;
	return ((size + mask) & ~(mask));
}

constexpr size_t PageAlign(const size_t size)
{
	return ByteAlign(size, PageSize);
}

template<typename T>
constexpr T MakePointer(const void* base, const ptrdiff_t offset = 0)
{
	return reinterpret_cast<T>((char*)base + offset);
}
