#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t CHAR16;

size_t mbstowcs(CHAR16* _Dest, char const* _Source, size_t _MaxCount);

#ifdef __cplusplus
}
#endif
