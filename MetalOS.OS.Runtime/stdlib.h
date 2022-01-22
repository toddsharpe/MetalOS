#pragma once

#include <crt_stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(noreturn) void exit(int _Code);
	void* calloc(size_t _Count, size_t _Size);
	void* realloc(void* _Block, size_t _Size);
	void* malloc(size_t _Size);
	void free(void* _Block);

	unsigned long long int strtoull(char const* _String, char** _EndPtr, int _Radix);
	unsigned long __cdecl strtoul(char const* _String, char** _EndPtr, int _Radix);
	long long __cdecl strtoll(char const* _String, char** _EndPtr, int _Radix);
	long double __cdecl strtold(char const* _String, char** _EndPtr);
	long __cdecl strtol(char const* _String, char** _EndPtr, int _Radix);
	float __cdecl strtof(char const* _String, char** _EndPtr);
	double __cdecl strtod(char const* _String, char** _EndPtr);

#ifdef __cplusplus
}
#endif
