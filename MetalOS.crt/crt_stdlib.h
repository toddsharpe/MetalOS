#pragma once

#include "MetalOS.crt.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t CHAR16;

size_t mbstowcs(CHAR16* _Dest, char const* _Source, size_t _MaxCount);
size_t wcstombs(char* _Dest, const CHAR16* _Source, size_t _MaxCount);

#define RAND_MAX 0x7fff
int rand();
void srand(unsigned int seed);

#ifdef __cplusplus
}
#endif

