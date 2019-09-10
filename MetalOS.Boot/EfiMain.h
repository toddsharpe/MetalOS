#pragma once

#include <efi.h>
#include <efilib.h>

EFI_STATUS PrintSystemTable(EFI_SYSTEM_TABLE* SystemTable);
EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);