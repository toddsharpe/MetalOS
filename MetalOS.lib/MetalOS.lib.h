#pragma once
#include <cstdarg>

extern void Print(const char* format, ...);
extern void Print(const char* format, va_list args);
extern void __declspec(noreturn) Bugcheck(const char* file, const char* line, const char* assert);
extern bool MetalOsLibDebug;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }

void LibPrint(const char* format, ...);

