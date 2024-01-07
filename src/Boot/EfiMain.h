#pragma once

#include <efi.h>
#include "LoaderParams.h"
#include "RamDrive.h"

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

EFI_STATUS PrintCpuDetails();
EFI_STATUS DisplayLoaderParams(const LoaderParams& params);
EFI_STATUS PrintDirectory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir);
EFI_STATUS PopulateDrive(RamDrive& drive, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir);

EFI_STATUS Keywait(const CHAR16* String = nullptr);

UINTN GetPhysicalAddressSize(const EFI_MEMORY_MAP& map);
EFI_STATUS DumpMemoryMap(const EFI_MEMORY_MAP& map);

extern EFI_MEMORY_TYPE AllocationType;
