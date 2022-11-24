#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(void const* _Buf1, void const* _Buf2, size_t _Size);
void* memcpy(void* _Dst, const void* _Src, size_t _Size);
void* memmove(void* _Dst, void const* _Src, size_t _Size);
void* memset(void* _Dst, int _Val, size_t _Size);
int strcmp(const char* str1, const char* str2);

size_t strlen(const char* str);

//ctype.h
#define __ascii_toupper(c)   ( (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c) )
int stricmp(char const* _String1, char const* _String2);

int strnicmp(char const* _String1, char const* _String2, size_t _MaxCount);

char* strcat(char* _Destination, char const* _Source);
char* strncat(char* _Destination, char const* _Source, size_t _Count);

int strncmp(char const* _Str1, char const* _Str2, size_t _MaxCount);

size_t strnlen_s(char const* _String, size_t _MaxCount);

char* strncpy(char* _Destination, char const* _Source, size_t _Count);

char* strcpy(char* _Destination, char const* _Source);

//char const* strchr(char* const _String, int const _Ch);
char * __cdecl strchr(     char const* _Str,       int         _Val);
//char* strrchr(char* const _String, int const _Ch);
char * __cdecl strrchr(
     char const* _Str,
       int         _Ch
);

#ifdef __cplusplus
}
#endif