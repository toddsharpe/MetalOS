#pragma once

#include <cstdint>

constexpr size_t DivRoundUp(const size_t x, const size_t y)
{
	return (x + y - 1) / y;
}

template <typename T>
constexpr T Max(const T a, const T b)
{
	return (a > b) ? a : b;
}

template <typename T>
constexpr T Min(const T a, const T b)
{
	return (a < b) ? a : b;
}

template <typename T>
constexpr T Clamp(const T v, const T min, const T max)
{
	return Min(Max(v, min), max);
}
