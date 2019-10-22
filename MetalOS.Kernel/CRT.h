#pragma once
#include "MetalOS.h"

class CRT
{
public:
	CRT() = delete;

	static void memcpy(void* dest, void* source, UINT32 size)
	{
		UINT64* dest_64 = (UINT64*)dest;
		UINT64* source_64 = (UINT64*)source;
		UINT32 chunkSize = 8;

		for (int i = 0; i < size / chunkSize; i++)
		{
			*dest_64 = *source_64;
			dest_64++;
			source_64++;
		}

		UINT8* dest_8 = (UINT8*)dest_64;
		UINT8* source_8 = (UINT8*)source_64;
		for (int i = 0; i < size % chunkSize; i++)
		{
			*dest_8 = *source_8;
			dest_8++;
			source_8++;
		}
	}

	static void memset(void* dest, UINT32 c, UINT32 size)
	{
		UINT64* dest_64 = (UINT64*)dest;
		UINT32 chunkSize = 8;

		for (int i = 0; i < size / chunkSize; i++)
		{
			*dest_64 = c;
			dest_64++;
		}

		UINT8* dest_8 = (UINT8*)dest_64;
		for (int i = 0; i < size % chunkSize; i++)
		{
			*dest_8 = c;
			dest_8++;
		}
	}

	static int memcmp(const void* ptr1, const void* ptr2, UINT32 num)
	{
		UINT64* ptr1_64 = (UINT64*)ptr1;
		UINT64* ptr2_64 = (UINT64*)ptr2;
		UINT32 chunkSize = 8;

		int i = 0;
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
};

