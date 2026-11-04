#include "core_crt/stdlib.h"
#include "core_crt/ctype.h"

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
