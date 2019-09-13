#include "String.h"

UINT32 efi_strlen(CHAR16* str)
{
	ASSERT(str != NULL);
	
	UINT32 length = 0;
	while (*str != L'\0')
	{
		length++;
		str++;
	}

	return length;
}

void efi_revstr(CHAR16* str)
{
	UINT32 length = efi_strlen(str);

	for (UINT32 i = 0; i < length / 2; i++)
	{
		CHAR16 temp = str[i];
		str[i] = str[length - i - 1];
		str[length - i - 1] = temp;
	}
}