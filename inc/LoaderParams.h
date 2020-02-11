#pragma once

//PixelsPerScanLine could be larger than HorizonalResolution if theres padding
typedef struct
{
	UINTN FrameBufferBase;
	UINT32 FrameBufferSize;
	UINT32 HorizontalResolution;
	UINT32 VerticalResolution;
	UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_DEVICE, * PEFI_GRAPHICS_DEVICE;

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
	UINT32 MemoryMapDescriptorVersion;

	//Page Tables
	EFI_PHYSICAL_ADDRESS PageTablesPoolAddress;
	UINT32 PageTablesPoolPageCount;

	EFI_GRAPHICS_DEVICE Display;

	//Config tables
	EFI_CONFIGURATION_TABLE* ConfigTables;
	UINTN ConfigTableSizes;
} LOADER_PARAMS, * PLOADER_PARAMS;

//KernelMain
typedef void (*KernelMain)(LOADER_PARAMS* params);