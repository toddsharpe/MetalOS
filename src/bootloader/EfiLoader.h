#pragma once

#include <efi.h>
#include <stdint.h>

class EfiLoader
{
public:
	static EFI_STATUS MapFile(EFI_FILE* file, EFI_PHYSICAL_ADDRESS& addressOut, size_t& sizeOut);
	static EFI_STATUS MapKernel(EFI_FILE* pFile, UINT64& imageSizeOut, UINT64& entryPointOut, EFI_PHYSICAL_ADDRESS& physicalImageBaseOut);
	static EFI_STATUS CrtInitialization(const uintptr_t imageBase);
};

