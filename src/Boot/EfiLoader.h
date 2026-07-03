#pragma once

#include <Efi.h>

namespace EfiLoader
{
	EFI_STATUS MapFile(EFI_FILE * file, const EFI_MEMORY_TYPE allocationType, EFI_PHYSICAL_ADDRESS& addressOut, size_t& sizeOut);

	//This method should check the memory map file and ensure nobody else has this reservation
	EFI_STATUS MapKernel(EFI_FILE* pFile, UINT64& imageSizeOut, UINT64& entryPointOut, EFI_PHYSICAL_ADDRESS& physicalImageBaseOut);

	//This function doesn't do any error checking, should it?
	EFI_STATUS CrtInitialization(const void* const imageBase);

	void* GetProcAddress(void* const imageBase, const CString& procName);
}
