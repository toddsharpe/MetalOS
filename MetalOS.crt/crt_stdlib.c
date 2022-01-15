#include "crt_stdlib.h"
#include "crt_string.h"
#include "crt_wchar.h"

//https://wiki.osdev.org/Random_Number_Generator
static unsigned long int next = 1;

int rand(void) // RAND_MAX assumed to be 32767
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) & RAND_MAX;
}

void srand(unsigned int seed)
{
	next = seed;
}

int abs(int n)
{
	return n > 0 ? n : -n;
}

int atoi(const char* str)
{
	int ret = 0;

	for (int i = 0; str[i] != '\0'; i++)
		ret = ret * 10 + (str[i] - '0');

	return ret;
}

size_t mbstowcs(CHAR16* _Dest, char const* _Source, size_t _MaxCount)
{
	size_t len = strlen(_Source);
	if (_MaxCount < len)
		len = _MaxCount;

	for (size_t i = 0; i < len; i++)
	{
		_Dest[i] = (CHAR16)_Source[i];
	}

	return len;
}

size_t wcstombs(char* _Dest, const CHAR16* _Source, size_t _MaxCount)
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
