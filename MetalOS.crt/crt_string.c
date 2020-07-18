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
	while ((*str1) && (*str1 == *str2))
	{
		str1++;
		str2++;
	}
	return *(unsigned char*)str1 - *(unsigned char*)str2;
}

//ctype.h
#define __ascii_toupper(c)   ( (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c) )
int stricmp(char const* _String1, char const* _String2)
{
	if (strlen(_String1) != strlen(_String2))
		return -1;

	for (size_t i = 0; i < strlen(_String1); i++) {
		if (__ascii_toupper(_String1[i]) != __ascii_toupper(_String2[i]))
			return _String1[i] - _String2[i];
	}
	return 0;
}

int strnicmp(char const* _String1, char const* _String2, size_t _MaxCount)
{
	for (size_t i = 0; i < strlen(_String1) && _MaxCount > 0; i++, _MaxCount--) {
		if (__ascii_toupper(_String1[i]) != __ascii_toupper(_String2[i]))
			return _String1[i] - _String2[i];
	}
	return 0;
}

char* strcat(char* _Destination, char const* _Source)
{
	char* s = _Destination + strlen(_Destination);
	
	while (*_Source != '\0')
		*s++ = *_Source++;
	*s = '\0';

	return _Destination;
}

char* strncat(char* _Destination, char const* _Source, size_t _Count)
{
	char* s = _Destination + strlen(_Destination);

	while (*_Source != '\0' && _Count--)
		*s++ = *_Source++;
	*s = '\0';

	return _Destination;
}

int strncmp(char const* _Str1, char const* _Str2, size_t _MaxCount)
{
	while ((*_Str1) && (*_Str1 == *_Str2) && _MaxCount--)
	{
		_Str1++;
		_Str2++;
	}
	if (_MaxCount)
		return *(unsigned char*)_Str1 - *(unsigned char*)_Str2;
	else
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

const char* strstr(char const* _String, char const* _SubString)
{
	char* s = _String;
	size_t length = strlen(_SubString);
	if (!length)
	{
		return s;
	}

	while (strlen(s) >= length)
	{
		if (strncmp(s, _SubString, length) == 0)
		{
			return (s);
		}
		s++;
	}

	return NULL;
}

const char* strchr(char* const _String, int const _Ch)
{
	char* s = _String;
	while (*s != (char)_Ch)
		if (!*s++)
			return 0;
	return (char*)s;
}

const char* strrchr(char* const _String, int const _Ch)
{
	char* s = _String;
	char* ret = 0;
	do {
		if (*s == (char)_Ch)
			ret = s;
	} while (*s++);
	return ret;
}
