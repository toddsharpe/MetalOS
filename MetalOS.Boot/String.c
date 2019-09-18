#include "String.h"
#include "Memory.h"

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

void efi_strrev(CHAR16* str)
{
	UINT32 length = efi_strlen(str);

	for (UINT32 i = 0; i < length / 2; i++)
	{
		CHAR16 temp = str[i];
		str[i] = str[length - i - 1];
		str[length - i - 1] = temp;
	}
}

//This should be dest, source
//TODO: Everyusage of strcpy in public code cant just be renamed, args have to be switched
//GDI
UINT32 efi_strcpy(CHAR16* source, CHAR16* dest)
{
	UINT32 length = efi_strlen(source);
	efi_memcpy(source, dest, efi_strlen(source) * sizeof(CHAR16));
	return length;
}

UINT32 efi_strcpy_n(CHAR16* source, CHAR16* dest, UINT32 count)
{
	efi_memcpy(source, dest, count * sizeof(CHAR16));
	return count;
}