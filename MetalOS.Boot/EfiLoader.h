#pragma once

#include <efi.h>
#include <efilib.h>

class EfiLoader
{
public:
	static EFI_STATUS MapFile(EFI_FILE* file, EFI_PHYSICAL_ADDRESS& address, size_t& size);
	static EFI_STATUS MapKernel(EFI_FILE* pFile, UINT64* pImageSizeOut, UINT64* pEntryPointOut, EFI_PHYSICAL_ADDRESS* pPhysicalImageBase);
	static EFI_STATUS CrtInitialization(UINT64 imageBase);
};

