#pragma once

#include <efi.h>
#include <efilib.h>
#include "MetalOS.Loader.h"

//UEFI memory map handling: reading the map, sizing physical memory, and
//pre-processing it into the simple ranges the kernel consumes.
namespace EfiMemory
{
	//Reads the current memory map into a static buffer.
	EFI_STATUS GetMemoryMap(EFI_MEMORY_MAP& map, UINTN& mapKey, UINT32& descriptorVersion);

	//Highest physical address (end of the last region), used to size the PFN DB.
	UINTN GetPhysicalAddressSize(const EFI_MEMORY_MAP& map);

	//Pre-processes the map into LoaderParams::FreeRanges/RuntimeRanges and assigns
	//runtime virtual addresses (KernelUefiStart + physical). Must run after
	//ExitBootServices; uses no boot services.
	void BuildRanges(EFI_MEMORY_MAP& map, LoaderParams& params);

	EFI_STATUS DumpMemoryMap(const EFI_MEMORY_MAP& map);
}
