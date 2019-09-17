#include "Memory.h"

// TODO: rewrite to work on UINT32 chunks
void efi_memcpy(void* source, void* dest, UINT32 count)
{
	UINT8* src_8 = (UINT8*)source;
	UINT8* dest_8 = (UINT8*)dest;

	while (count > 0)
	{
		*dest_8 = *src_8;
		src_8++;
		dest_8++;
		count--;
	}
}

void efi_memset(void* dest, UINT8 value, UINT32 count)
{
	UINT8* dest_8 = (UINT8*)dest;

	while (count > 0)
	{
		*dest_8 = value;
		dest_8++;
		count--;
	}
}

int efi_memcmp(void* ptr1, void* ptr2, UINT32 count)
{
	UINT8* ptr1_8 = (UINT8*)ptr1;
	UINT8* ptr2_8 = (UINT8*)ptr2;

	while (count > 0)
	{
		UINT8 val1_8_i = *ptr1_8;
		UINT8 val2_8_i = *ptr2_8;

		if (val1_8_i < val2_8_i)
		{
			return -1;
		}
		else if (val2_8_i > val1_8_i)
		{
			return 1;
		}

		count--;
		ptr1_8++;
		ptr2_8++;
	}

	return 0;
}