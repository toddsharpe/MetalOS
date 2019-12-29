#pragma once

#include "Kernel.h"
//Shared header between loader and kernel

//PixelsPerScanLine could be larger than HorizonalResolution if theres padding

//No point in supporting multiple monitors since this is built for hyper-v
typedef struct
{
	//Kernel details
	EFI_PHYSICAL_ADDRESS KernelAddress;
	UINT64 KernelImageSize;

	//UEFI runtime services
	EFI_RUNTIME_SERVICES* Runtime;

	//MemoryMap - allocated on its own page
	EFI_MEMORY_DESCRIPTOR* MemoryMap;
	UINTN MemoryMapSize;
	UINTN MemoryMapDescriptorSize;
	UINT32 MemoryMapVersion;

	//Page Tables
	EFI_PHYSICAL_ADDRESS PageTablesPoolAddress;
	UINT32 PageTablesPoolPageCount;

	GRAPHICS_DEVICE Display;

	//Config tables
	EFI_CONFIGURATION_TABLE* ConfigTables;
	UINTN ConfigTableSizes;
} LOADER_PARAMS, *PLOADER_PARAMS;

//KernelMain
typedef void (*KernelMain)(LOADER_PARAMS* params);


