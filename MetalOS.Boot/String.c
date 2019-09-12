#include "String.h"

UINT32 efi_strlen(CHAR16* str)
{
	ASSERT(string != NULL);
	
	UINT32 length = 0;
	while (*str != L'\0')
	{
		length++;
		str++;
	}

	return length;
}
