#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include <efilib.h>
#include <LoaderParams.h>
#include <RamDrive.h>

#include "MetalOS.Internal.h"

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

EFI_STATUS PrintCpuDetails();
EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* pParams);
EFI_STATUS PrintGraphicsDevice(PEFI_GRAPHICS_DEVICE pDevice);
EFI_STATUS DumpMemoryMap();
EFI_STATUS PrintDirectory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir);
EFI_STATUS PopulateDrive(RamDrive& drive, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir);

void Print(const char* format, ...);
void Print(const char* format, va_list args);
EFI_STATUS Print(const CHAR16* format, ...);
 void UartPrint(const char* format, ...);
 void UartPrint(const CHAR16* format, ...);
EFI_STATUS Keywait(const CHAR16* String = nullptr);

UINTN GetPhysicalAddressSize(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, EFI_MEMORY_DESCRIPTOR* MemoryMap);
