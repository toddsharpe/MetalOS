#pragma once

#include <efi.h>
#include <efilib.h>

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle);
EFI_STATUS MapFile(EFI_FILE* file, EFI_PHYSICAL_ADDRESS* imageBaseOut, EFI_PHYSICAL_ADDRESS* entryPoint);