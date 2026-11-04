#include "core_crt/stdlib.h"
#include "core_crt/ctype.h"
#include "core_crt/wchar.h"
#include <cstring>

// https://wiki.osdev.org/Random_Number_Generator
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

int atoi(const char *str)
{
	//Leading whitespace, an optional sign, then digits up to the first non-digit, as C specifies
	while (isspace((unsigned char)*str))
		str++;

	const int negative = (*str == '-');
	if (*str == '-' || *str == '+')
		str++;

	int ret = 0;
	for (; *str >= '0' && *str <= '9'; str++)
		ret = ret * 10 + (*str - '0');

	return negative ? -ret : ret;
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
