#pragma once

#include <Efi.h>

namespace EfiLoader
{
	//This method should check the memory map file and ensure nobody else has this reservation
	EFI_STATUS MapKernel(EFI_FILE* pFile, UINT64& imageSizeOut, UINT64& entryPointOut, EFI_PHYSICAL_ADDRESS& physicalImageBaseOut);

	//This function doesn't do any error checking, should it?
	EFI_STATUS CrtInitialization(const void* const imageBase);
}
