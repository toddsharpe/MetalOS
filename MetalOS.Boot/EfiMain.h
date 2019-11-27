#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include <efilib.h>

#include "Kernel.h"
#include "LoaderParams.h"

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
EFI_STATUS Keywait(const CHAR16* String = nullptr);
EFI_STATUS DisplayLoaderParams(PLOADER_PARAMS pParams);
EFI_STATUS PrintCpuDetails();
EFI_STATUS DumpGDT();
