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