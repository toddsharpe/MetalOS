#pragma once

#include <efi.h>
#include <efilib.h>

#include "LoaderParams.h"

EFI_STATUS Keywait(CHAR16* String);
EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* params);