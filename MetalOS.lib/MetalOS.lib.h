#pragma once
#include <cstdarg>

void Print(const char* format, ...);
void Print(const char* format, va_list args);
extern bool MetalOsLibDebug;

void LibPrint(const char* format, ...);

