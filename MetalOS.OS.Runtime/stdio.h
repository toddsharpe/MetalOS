#pragma once

#include <crt_stdio.h>

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

#define stdin 0
#define stdout 1
#define stderr 2

#define printf DebugPrintf

typedef void FILE;

LIBRARY FILE* fopen(char const* _FileName, char const* _Mode);
LIBRARY size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream);
LIBRARY int fseek(FILE* _Stream, long _Offset, int _Origin);
LIBRARY int fclose(FILE* _Stream);

int LIBRARY fprintf(FILE* const _Stream, char const* const _Format, ...);

