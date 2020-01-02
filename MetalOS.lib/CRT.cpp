#include "CRT.h"

namespace crt
{
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
		//TODO: optimize for UINT32/64?
		uint8_t* dest_8 = (uint8_t*)_Dst;
		const uint8_t* source_8 = (uint8_t*)_Src;
		for (int i = _Size - 1; i >= 0; i--)
		{
			dest_8[i] = source_8[i];
		}

		return _Dst;
	}

	uint32_t strcpy(CHAR16* dest, const CHAR16* source)
	{
		size_t length = strlen(source);
		memcpy(dest, source, strlen(source) * sizeof(CHAR16));
		return length;
	}

	size_t strlen(const CHAR16* str)
	{
		size_t length = 0;
		while (*str != L'\0')
		{
			length++;
			str++;
		}

		return length;
	}

	uint32_t strncpy(CHAR16* dest, const CHAR16* source, uint32_t num)
	{
		memcpy(dest, source, num * sizeof(CHAR16));
		return num;
	}

	int strcmp(const CHAR16* str1, const CHAR16* str2)
	{
		return memcmp(str1, str2, strlen(str1));
	}

	void strrev(CHAR16* str)
	{
		size_t length = strlen(str);

		for (size_t i = 0; i < length / 2; i++)
		{
			CHAR16 temp = str[i];
			str[i] = str[length - i - 1];
			str[length - i - 1] = temp;
		}
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

	int strcmp(const char* str1, const char* str2)
	{
		return memcmp(str1, str2, crt::strlen(str1));
	}

	void GetDirectoryName(const CHAR16* source, CHAR16* destination)
	{
		size_t current = 0;
		size_t marker = 0;
		while (source[current] != L'\0')
		{
			if (source[current] == L'\\')
				marker = current;
			current++;
		}

		//Include trailing slash
		marker++;

		//Copy to destination
		crt::strncpy(destination, source, marker);

		//Add null
		destination[marker + 1] = L'\0';
	}
}

//C function prototypes
//TODO: revisit?
extern "C" void* memcpy(void* _Dst, const void* _Src, size_t _Size)
{
	return crt::memcpy(_Dst, _Src, _Size);
}

extern "C" void* memset(void* _Dst, int _Val, size_t _Size)
{
	return crt::memset(_Dst, _Val, _Size);
}

extern "C" void* memmove(void* _Dst, void const* _Src, size_t _Size)
{
	return crt::memmove(_Dst, _Src, _Size);
}
