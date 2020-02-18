#pragma once

#ifdef DEBUG_LIB
extern "C" void Print(const char* format, ...);
#else
#define Print(x)
#endif
