#pragma once

#include <cstdarg>

EFI_STATUS UartPrintf(const CHAR16* format, ...);
void UartPrintf(const char* format, ...);
EFI_STATUS Print(const CHAR16* format, ...);
void Printf(const char* format, ...);
void Bugcheck(const char* file, const char* line, const char* format, ...);
