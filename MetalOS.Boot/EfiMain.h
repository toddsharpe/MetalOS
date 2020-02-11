#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include <efilib.h>
#include <LoaderParams.h>

#include "MetalOS.Internal.h"

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
EFI_STATUS Keywait(const CHAR16* String = nullptr);
EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* pParams);
EFI_STATUS PrintCpuDetails();
UINTN GetPhysicalAddressSize(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, EFI_MEMORY_DESCRIPTOR* MemoryMap);
