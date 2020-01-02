#pragma once

#include <cstdint>
#include <cstdarg>

class String
{
public:
	static uint32_t strlen(const char* cStr);
	static int sprintf(char* buf, const char* cfmt, ...);
	static int kvprintf(char const* fmt, void (*func)(int, void*), void* arg, int radix, va_list ap);
	static char* ksprintn(char* nbuf, uintmax_t num, int base, int* len, int upper);
	static int vsprintf(char* buf, const char* cfmt, va_list ap);

private:
	static inline int imax(int a, int b)
	{
		return (a > b ? a : b);
	}
};

