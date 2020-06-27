#include "crt_string.h"

int memcmp(void const* _Buf1, void const* _Buf2, size_t _Size)
{
	uintptr_t* ptr1_64 = (uintptr_t*)_Buf1;
	uintptr_t* ptr2_64 = (uintptr_t*)_Buf2;

	size_t i = 0;
	while (i < _Size / sizeof(uintptr_t))
	{
		if (*ptr1_64 < *ptr2_64)
			return -1;
		else if (*ptr1_64 > * ptr2_64)
			return 1;

		i++;
		ptr1_64++;
		ptr2_64++;
	}

	uint8_t* ptr1_8 = (uint8_t*)ptr1_64;
	uint8_t* ptr2_8 = (uint8_t*)ptr2_64;

	i = 0;
	while (i < _Size % sizeof(uintptr_t))
	{
		if (*ptr1_8 < *ptr2_8)
			return -1;
		else if (*ptr1_8 > * ptr2_8)
			return 1;

		i++;
		ptr1_8++;
		ptr2_8++;
	}

	return 0;
}

void* memcpy(void* _Dst, const void* _Src, size_t _Size)
{
	uintptr_t* dest = (uintptr_t*)_Dst;
	uintptr_t* source = (uintptr_t*)_Src;

	for (size_t i = 0; i < _Size / sizeof(uintptr_t); i++)
	{
		*dest = *source;
		dest++;
		source++;
	}

	uint8_t* dest_8 = (uint8_t*)dest;
	uint8_t* source_8 = (uint8_t*)source;
	for (size_t i = 0; i < _Size % sizeof(uintptr_t); i++)
	{
		*dest_8 = *source_8;
		dest_8++;
		source_8++;
	}

	return _Dst;
}

void* memmove(void* _Dst, void const* _Src, size_t _Size)
{
	//https://www.student.cs.uwaterloo.ca/~cs350/common/os161-src-html/doxygen/html/memmove_8c_source.html
	/*
	 * If the destination is below the source, we have to copy
	 * front to back.
	 *
	 *      dest:   dddddddd
	 *      src:    ^   ssssssss
	 *              |___|  ^   |
	 *                     |___|
	*/
	if ((uintptr_t)_Dst < (uintptr_t)_Src)
		return memcpy(_Dst, _Src, _Size);

	/*
	 * If the destination is above the source, we have to copy
	 * back to front to avoid overwriting the data we want to
	 * copy.
	 *
	 *      dest:       dddddddd
	 *      src:    ssssssss   ^
	 *              |   ^  |___|
	 *              |___|
	 *
	*/

	uint8_t* dest_8 = (uint8_t*)_Dst;
	const uint8_t* source_8 = (uint8_t*)_Src;
	for (int i = _Size - 1; i >= 0; i--)
	{
		dest_8[i] = source_8[i];
	}

	return _Dst;
}

void* memset(void* _Dst, int _Val, size_t _Size)
{
	const uint8_t value_8 = (uint8_t)_Val;
	uintptr_t* dest_64 = (uintptr_t*)_Dst;

	for (size_t i = 0; i < _Size / sizeof(uintptr_t); i++)
	{
		*dest_64 = value_8;
		dest_64++;
	}

	uint8_t* dest_8 = (uint8_t*)dest_64;
	for (size_t i = 0; i < _Size % sizeof(uintptr_t); i++)
	{
		*dest_8 = value_8;
		dest_8++;
	}

	return _Dst;
}

int strcmp(const char* str1, const char* str2)
{
	return memcmp(str1, str2, strlen(str1));
}

//ctype.h
#define __ascii_toupper(c)   ( (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c) )
int stricmp(char const* _String1, char const* _String2)
{
	int i;

	if (strlen(_String1) != strlen(_String2))
		return -1;

	for (i = 0; i < strlen(_String1); i++) {
		if (__ascii_toupper(_String1[i]) != __ascii_toupper(_String2[i]))
			return _String1[i] - _String2[i];
	}
	return 0;
}

size_t strlen(const char* str)
{
	size_t length = 0;
	while (*str != '\0')
	{
		length++;
		str++;
	}

	return length;
}

size_t strnlen_s(char const* _String, size_t _MaxCount)
{
	if (_String == NULL) {
		return 0;
	}

	size_t count = 0;

	while (*_String++ && _MaxCount--) {
		count++;
	}

	return count;
}

char* strcpy(char* _Destination, char const* _Source)
{
	return strncpy(_Destination, _Source, strlen(_Source));
}

char* strncpy(char* _Destination, char const* _Source, size_t _Count)
{
	memcpy(_Destination, _Source, _Count * sizeof(char));
	return _Destination;
}
