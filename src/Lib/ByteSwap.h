#pragma once

#include <cstdint>
#include "x64/intrin.h"

template <typename T>
T ByteSwap(const T value);

template <>
inline char ByteSwap<char>(const char value)
{
	return value;
}

template <>
inline bool ByteSwap<bool>(const bool value)
{
	return value;
}

template <>
inline uint8_t ByteSwap<uint8_t>(const uint8_t value)
{
	return value;
}

template <>
inline int8_t ByteSwap<int8_t>(const int8_t value)
{
	return value;
}

template <>
inline uint16_t ByteSwap<uint16_t>(const uint16_t value)
{
	return _byteswap_ushort(value);
}

template <>
inline int16_t ByteSwap<int16_t>(const int16_t value)
{
	return _byteswap_ushort(value);
}

template <>
inline uint32_t ByteSwap<uint32_t>(const uint32_t value)
{
	return _byteswap_ulong(value);
}

template <>
inline int32_t ByteSwap<int32_t>(const int32_t value)
{
	return _byteswap_ulong(value);
}

template <>
inline uint64_t ByteSwap<uint64_t>(const uint64_t value)
{
	return _byteswap_uint64(value);
}

template <>
inline int64_t ByteSwap<int64_t>(const int64_t value)
{
	return _byteswap_uint64(value);
}

template <>
inline float ByteSwap<float>(const float value)
{
	uint32_t temp = 0;
	memcpy(&temp, &value, sizeof(value));

	temp = _byteswap_ulong(temp);

	float swapped = 0;
	memcpy(&swapped, &temp, sizeof(value));
	return swapped;
}

template <>
inline double ByteSwap<double>(const double value)
{
	uint64_t temp = 0;
	memcpy(&temp, &value, sizeof(value));

	temp = _byteswap_uint64(temp);

	double swapped = 0;
	memcpy(&swapped, &temp, sizeof(value));
	return swapped;
}
