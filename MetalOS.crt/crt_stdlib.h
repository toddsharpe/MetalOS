#pragma once

#include "MetalOS.crt.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t CHAR16;

//LIBRARY size_t mbstowcs(CHAR16* _Dest, char const* _Source, size_t _MaxCount);
//LIBRARY size_t wcstombs(char* _Dest, const CHAR16* _Source, size_t _MaxCount);

#define RAND_MAX 0x7fff
LIBRARY int rand();
LIBRARY void srand(unsigned int seed);
LIBRARY int abs(int n);
LIBRARY int atoi(const char* str);

#ifdef __cplusplus
}
#endif

