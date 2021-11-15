#pragma once

#include <MetalOS.crt.h>
#include <crt_string.h>

#define _strdup strdup

#ifdef __cplusplus
extern "C"
{
#endif

LIBRARY char* strdup(char const* _String);

#ifdef __cplusplus
}
#endif
