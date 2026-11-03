#include "core_crt/wchar.h"

int wcscmp(const wchar_t *str1, const wchar_t *str2)
{
	while ((*str1) && (*str1 == *str2))
	{
		str1++;
		str2++;
	}
	return *(uint16_t *)str1 - *(uint16_t *)str2;
}

size_t wcslen(const wchar_t *str)
{
	size_t length = 0;
	while (*str != L'\0')
	{
		length++;
		str++;
	}

	return length;
}

wchar_t *wcscpy(wchar_t *dest, const wchar_t *source)
{
	size_t length = wcslen(source);
	memcpy(dest, source, length * sizeof(wchar_t));
	return dest;
}

wchar_t *wcsncpy(wchar_t *dest, const wchar_t *source, size_t num)
{
	memcpy(dest, source, num * sizeof(wchar_t));
	return dest;
}
