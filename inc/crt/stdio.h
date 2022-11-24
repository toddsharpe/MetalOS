#pragma once

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int sprintf(char* const _Buffer, char const* const _Format, ...);
int snprintf(char* const _Buffer, size_t const _BufferCount, char const* const _Format, ...);
int vsprintf(char* const _Buffer, char const* const _Format, va_list _ArgList);
int vsnprintf(char* const _Buffer, size_t const _BufferCount, char const* const _Format, va_list _ArgList);
char* ksprintn(char* nbuf, uintmax_t num, int base, int* lenp, int upper);
int kvprintf(char const* fmt, void (*func)(int, void*), void* arg, int radix, va_list ap);

#ifdef __cplusplus
}
#endif