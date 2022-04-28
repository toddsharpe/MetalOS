#pragma once

#include <crt_stdio.h>

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

#define stdin 0
#define stdout 1
#define stderr 2

#define BUFSIZ  512

#define EOF    (-1)

//#define printf DebugPrintf

#define _FILE_DEFINED
typedef void FILE;

typedef __int64 fpos_t;

#ifdef __cplusplus
extern "C"
{
#endif

LIBRARY FILE* fopen(char const* _FileName, char const* _Mode);
LIBRARY size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream);
LIBRARY size_t fwrite(void const* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream);
LIBRARY int fseek(FILE* _Stream, long _Offset, int _Origin);
LIBRARY int ftell(FILE* _Stream);
LIBRARY int fclose(FILE* _Stream);

LIBRARY int fprintf(FILE* const _Stream, char const* const _Format, ...);
LIBRARY int vfprintf(FILE* const _Stream, char const* const _Format, va_list _ArgList);

LIBRARY int __cdecl rename(char const* _OldFileName, char const* _NewFileName);
LIBRARY int __cdecl remove(char const* _FileName);

int __cdecl _scprintf(char const* const _Format, ...);
int __cdecl sprintf_s(char* const _Buffer, size_t const _BufferCount, char const* const _Format, ...);

LIBRARY int printf(char const* const _Format, ...);

#ifdef __cplusplus
}
#endif
