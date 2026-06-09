#include "core_crt/stdint.h"

#define RAND_MAX 0x7fff

#ifdef __cplusplus
extern "C"
{
#endif

	// https://wiki.osdev.org/Random_Number_Generator
	int rand(void);

	void srand(unsigned int seed);

	int abs(int n);

	int atoi(const char *str);

	size_t mbstowcs(wchar_t * _Dest, char const *_Source, size_t _MaxCount);

	size_t wcstombs(char *_Dest, const wchar_t *_Source, size_t _MaxCount);

	void *malloc( size_t size );
	void free(void* ptr);

#ifdef __cplusplus
}
#endif
