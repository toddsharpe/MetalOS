#include "CRT.h"

void* CRT::memcpy(void* dest, const void* source, UINT32 size)
{
	UINT64* dest_64 = (UINT64*)dest;
	UINT64* source_64 = (UINT64*)source;
	UINT32 chunkSize = 8;

	for (size_t i = 0; i < size / chunkSize; i++)
	{
		*dest_64 = *source_64;
		dest_64++;
		source_64++;
	}

	UINT8* dest_8 = (UINT8*)dest_64;
	UINT8* source_8 = (UINT8*)source_64;
	for (size_t i = 0; i < size % chunkSize; i++)
	{
		*dest_8 = *source_8;
		dest_8++;
		source_8++;
	}

	return dest;
}

void* CRT::memset(void* dest, UINT32 c, UINT32 size)
{
	UINT64* dest_64 = (UINT64*)dest;
	UINT32 chunkSize = 8;

	for (size_t i = 0; i < size / chunkSize; i++)
	{
		*dest_64 = c;
		dest_64++;
	}

	UINT8* dest_8 = (UINT8*)dest_64;
	for (size_t i = 0; i < size % chunkSize; i++)
	{
		*dest_8 = c;
		dest_8++;
	}

	return dest;
}

int CRT::memcmp(const void* ptr1, const void* ptr2, UINT32 num)
{
	UINT64* ptr1_64 = (UINT64*)ptr1;
	UINT64* ptr2_64 = (UINT64*)ptr2;
	UINT32 chunkSize = 8;

	size_t i = 0;
	while (i < num / chunkSize)
	{
		if (*ptr1_64 < *ptr2_64)
			return -1;
		else if (*ptr1_64 > * ptr2_64)
			return 1;

		i++;
		ptr1_64++;
		ptr2_64++;
	}

	UINT8* ptr1_8 = (UINT8*)ptr1_64;
	UINT8* ptr2_8 = (UINT8*)ptr2_64;

	i = 0;
	while (i < num % chunkSize)
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

void* CRT::memmove(void* dest, const void* source, UINT32 size)
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
	if ((UINT64)dest < (UINT64)source)
		return CRT::memcpy(dest, source, size);

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
	UINT8* dest_8 = (UINT8*)dest;
	const UINT8* source_8 = (UINT8*)source;
	for (int i = size - 1; i >= 0; i--)
	{
		dest_8[i] = source_8[i];
	}

	return dest;
}

UINT32 CRT::strcpy(CHAR16* dest, const CHAR16* source)
{
	UINT32 length = CRT::strlen(source);
	CRT::memcpy(dest, source, CRT::strlen(source) * sizeof(CHAR16));
	return length;
}

UINT32 CRT::strlen(const CHAR16* str)
{
	UINT32 length = 0;
	while (*str != L'\0')
	{
		length++;
		str++;
	}

	return length;
}

UINT32 CRT::strncpy(CHAR16* dest, const CHAR16* source, UINT32 num)
{
	CRT::memcpy(dest, source, num * sizeof(CHAR16));
	return num;
}

UINT32 CRT::strcmp(const CHAR16* str1, const CHAR16* str2)
{
	return CRT::memcmp(str1, str2, CRT::strlen(str1));
}

void CRT::strrev(CHAR16* str)
{
	UINT32 length = CRT::strlen(str);

	for (UINT32 i = 0; i < length / 2; i++)
	{
		CHAR16 temp = str[i];
		str[i] = str[length - i - 1];
		str[length - i - 1] = temp;
	}
}

UINT32 CRT::strlen(const char* str)
{
	UINT32 length = 0;
	while (*str != '\0')
	{
		length++;
		str++;
	}

	return length;
}

UINT32 CRT::strcmp(const char* str1, const char* str2)
{
	return CRT::memcmp(str1, str2, CRT::strlen(str1));
}

void CRT::GetDirectoryName(const CHAR16* source, CHAR16* destination)
{
	UINT32 current = 0;
	UINT32 marker = 0;
	while (source[current] != L'\0')
	{
		if (source[current] == L'\\')
			marker = current;
		current++;
	}

	//Include trailing slash
	marker++;

	//Copy to destination
	CRT::strncpy(destination, source, marker);

	//Add null
	destination[marker + 1] = L'\0';
}

//C function prototypes
//TODO: remove CRT functions from class?
extern "C" void* memmove(void* dest, const void* source, UINT32 size)
{
	return CRT::memmove(dest, source, size);
}

extern "C" void* memcpy(void* dest, const void* source, UINT32 size)
{
	return CRT::memcpy(dest, source, size);
}

extern "C" void* memset(void* dest, UINT32 c, UINT32 size)
{
	return CRT::memset(dest, c, size);
}

