#pragma once

#include <efi.h>
#include <efilib.h>

extern EFI_SYSTEM_TABLE* ST;
extern EFI_RUNTIME_SERVICES* RT;
extern EFI_BOOT_SERVICES* BS;

EFI_STATUS Keywait(CHAR16* String);
EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);