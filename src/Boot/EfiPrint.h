#pragma once

#include <cstdarg>

EFI_STATUS UartPrintf(const char* format, ...);
EFI_STATUS Print(const char* format, ...);
void Printf(const char* format, ...);
void VPrintf(const char* format, va_list args);
void Bugcheck(const char* file, const char* line, const char* format, ...);
