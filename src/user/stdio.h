#pragma once

#include "core_crt/stdarg.h"
#include "core_crt/stdio.h"

typedef void* FILE;
/* Seek method constants */

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

/* Buffered I/O macros */

#define BUFSIZ  512

#define stdin  ((FILE*)0)
#define stdout ((FILE*)1)
#define stderr ((FILE*)2)

#ifdef __cplusplus
extern "C"
{
#endif

	FILE* fopen(char const* _FileName, char const* _Mode);

	size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream);

	size_t fwrite(void const* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream);

	int fseek(FILE* _Stream, long _Offset, int _Origin);

	long ftell(FILE* _Stream);

	int fclose(FILE* _Stream);

	int vfprintf(FILE* const _Stream, char const* const _Format, va_list _ArgList);

	int fprintf(FILE* const _Stream, char const* const _Format, ...);

	int fflush(FILE *stream);

	int rename(char const* _OldFileName, char const* _NewFileName);

	int remove(char const* _FileName);

	int printf(char const* const _Format, ...);
	int puts ( const char * str );
	int putchar ( int character );
	int sscanf ( const char * s, const char * format, ...);

#ifdef __cplusplus
}
#endif
