#pragma once

#include "MetalOS.crt.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//Memory Routines
LIBRARY int memcmp(void const* _Buf1, void const* _Buf2, size_t _Size);
LIBRARY void* memcpy(void* _Dst, const void* _Src, size_t _Size);
LIBRARY void* memmove(void* _Dst, void const* _Src, size_t _Size);
LIBRARY void* memset(void* _Dst, int _Val, size_t _Size);

//String
LIBRARY int strcmp(char const* _Str1, char const* _Str2);
LIBRARY int stricmp(char const* _String1, char const* _String2);
LIBRARY int strnicmp(char const* _String1,char const* _String2, size_t _MaxCount);
LIBRARY char* strcat(char* _Destination, char const* _Source);
LIBRARY char* strncat(char* _Destination, char const* _Source, size_t _Count);
LIBRARY int strncmp(char const* _Str1, char const* _Str2, size_t _MaxCount);
LIBRARY size_t strlen(char const* _Str);
LIBRARY size_t strnlen_s(char const* _String, size_t _MaxCount);
LIBRARY char* strcpy(char* _Destination, char const* _Source);
LIBRARY char* strncpy(char* _Destination, char const* _Source, size_t _Count);
//LIBRARY const char* strstr(char const* _String, char const* _SubString);
//LIBRARY const char* strchr(char* const _String, int const _Ch);
//LIBRARY const char* strrchr(char* const _String, int const _Ch);

#ifdef __cplusplus
}
#endif
