#pragma once

#include <efi.h>

EFI_STATUS Print(const CHAR16* format, ...);
EFI_STATUS UartPrintf(const CHAR16* format, ...);
void UartPrintf(const char* format, ...);
