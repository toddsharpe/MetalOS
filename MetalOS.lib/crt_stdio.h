#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int crt_sprintf(char* const _Buffer, char const* const _Format, ...);
int crt_vsprintf(char* const _Buffer, char const* const _Format, va_list _ArgList); //TODO: vector includes this header and MSVC chooses it for some reason.

#ifdef __cplusplus
}
#endif
