#pragma once

#include "MetalOS.crt.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

LIBRARY int sprintf(char* const _Buffer, char const* const _Format, ...);
LIBRARY int snprintf(char* const _Buffer, size_t const _BufferCount, char const* const _Format, ...);
LIBRARY int vsprintf(char* const _Buffer, char const* const _Format, va_list _ArgList);

#ifdef __cplusplus
}
#endif
