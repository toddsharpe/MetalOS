#include "core_crt/wchar.h"
#include "core_crt/string.h"

//Intrinsics under /Oi, implemented here
#pragma function(wcscmp, wcslen, wcscpy)

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
	//+1 so the terminating null is copied too
	size_t length = wcslen(source);
	memcpy(dest, source, (length + 1) * sizeof(wchar_t));
	return dest;
}

wchar_t *wcsncpy(wchar_t *dest, const wchar_t *source, size_t num)
{
	//Copies up to the source's terminator and pads the rest of num with NULs, never reading past the source
	size_t i = 0;
	for (; i < num && source[i]; i++)
		dest[i] = source[i];
	for (; i < num; i++)
		dest[i] = L'\0';
	return dest;
}

size_t mbstowcs(wchar_t *_Dest, char const *_Source, size_t _MaxCount)
{
	size_t len = strlen(_Source);
	if (_MaxCount < len)
		len = _MaxCount;

	for (size_t i = 0; i < len; i++)
	{
		_Dest[i] = (wchar_t)_Source[i];
	}

	return len;
}

size_t wcstombs(char *_Dest, const wchar_t *_Source, size_t _MaxCount)
{
	const size_t len = wcslen(_Source);
	if (_MaxCount < len)
		return (size_t)-1;

	for (size_t i = 0; i < len; i++)
	{
		_Dest[i] = (char)_Source[i];
	}

	return len;
}
