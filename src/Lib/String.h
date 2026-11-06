#pragma once

#include <cstdint>
#include <cstring>
#include "Lib/Span.h"

//Strings are stored in char arrays, always null-terminated, and passed around as cstr_t

//Length of a C string, stopping at max chars
constexpr size_t StrLength(const char* const text, const size_t max = (size_t)-1)
{
	size_t length = 0;
	while (length < max && text[length] != '\0')
		length++;
	return length;
}

//The text in a char array, up to its terminator or the end of the array
template <size_t N>
constexpr cstr_t Str(const char (&text)[N])
{
	return { text, StrLength(text, N) };
}

//A null-terminated C string, read at most max chars
constexpr cstr_t CStr(const char* const text, const size_t max = (size_t)-1)
{
	return { text, text ? StrLength(text, max) : 0 };
}

//Copies text into a char array, clipped to fit, and terminates it; returns the length kept
template <size_t N>
size_t StrCopy(char (&to)[N], const cstr_t text)
{
	const size_t length = text.Size < N - 1 ? text.Size : N - 1;
	memcpy(to, text.Data, length);
	to[length] = '\0';
	return length;
}

//Appends text to the string in a char array, clipped to fit; returns the new length
template <size_t N>
size_t StrAppend(char (&to)[N], const cstr_t text)
{
	const size_t used = StrLength(to, N - 1);
	const size_t length = text.Size < N - 1 - used ? text.Size : N - 1 - used;
	memcpy(to + used, text.Data, length);
	to[used + length] = '\0';
	return used + length;
}
