#pragma once

#include <crt_string.h>

#define _strdup strdup

#ifdef __cplusplus
extern "C" {
#endif

char* strdup(char const* _String);

#ifdef __cplusplus
}
#endif

