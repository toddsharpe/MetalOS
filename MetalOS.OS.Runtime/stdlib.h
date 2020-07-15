#pragma once

#include <crt_stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t _Size);
void free(void* _Block);

#ifdef __cplusplus
}
#endif
