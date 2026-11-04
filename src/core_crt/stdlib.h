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

	void *malloc( size_t size );
	void free(void* ptr);

#ifdef __cplusplus
}
#endif
