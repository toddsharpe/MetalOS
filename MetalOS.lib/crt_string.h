#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//Memory Routines
int memcmp(void const* _Buf1, void const* _Buf2, size_t _Size);
void* memcpy(void* _Dst, const void* _Src, size_t _Size);
void* memmove(void* _Dst, void const* _Src, size_t _Size);
void* memset(void* _Dst, int _Val, size_t _Size);

//String
int strcmp(char const* _Str1, char const* _Str2);
size_t strlen(char const* _Str);
size_t strnlen_s(char const* _String, size_t _MaxCount);
char* strncpy(char* _Destination, char const* _Source, size_t _Count);

#ifdef __cplusplus
}
#endif
