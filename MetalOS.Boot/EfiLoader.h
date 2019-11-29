#pragma once

#include <efi.h>
#include <efilib.h>

class EfiLoader
{
public:
	static EFI_STATUS MapKernel(EFI_FILE* file, UINT64* pImageSizeOut, UINT64* pEntryPointOut, EFI_PHYSICAL_ADDRESS* pPhysicalImageBase);
	static EFI_STATUS CrtInitialization(UINT64 imageBase);
};

