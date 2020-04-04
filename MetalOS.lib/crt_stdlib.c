#include "crt_stdlib.h"
#include "crt_string.h"

//https://wiki.osdev.org/Random_Number_Generator
static unsigned long int next = 1;

int rand(void) // RAND_MAX assumed to be 32767
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % (RAND_MAX + 1);
}

void srand(unsigned int seed)
{
	next = seed;
}

size_t mbstowcs(CHAR16* _Dest, char const* _Source, size_t _MaxCount)
{
	const size_t len = strlen(_Source);
	if (_MaxCount < len * sizeof(CHAR16))
		return (size_t)-1;

	for (size_t i = 0; i < len; i++)
	{
		_Dest[i] = (CHAR16)_Source[i];
	}

	return len;
}
