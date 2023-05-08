#pragma once

#include <stdint.h>
#include <efi.h>
#include <kernel/MetalOS.List.h>

//PixelsPerScanLine could be larger than HorizonalResolution if theres padding
typedef struct
{
	UINTN FrameBufferBase;
	UINTN FrameBufferSize;
	UINT32 HorizontalResolution;
	UINT32 VerticalResolution;
	UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_DEVICE, * PEFI_GRAPHICS_DEVICE;

enum class PageState
{
	Free,
	Active,
	Platform //Used for acpi/runtime/reserved pages
};

//Simple struct to represent a physical page. Its state and list pointers
typedef struct _PAGE_FRAME
{
	//Pointers when placing frames on the lists
	ListEntry Link;
	PageState State;
} PAGE_FRAME;

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

	//PhysicaMemoryManager
	EFI_PHYSICAL_ADDRESS PfnDbAddress;
	size_t PfnDbSize;

	EFI_GRAPHICS_DEVICE Display;

	//Config tables
	EFI_CONFIGURATION_TABLE* ConfigTables;
	UINTN ConfigTableSizes;

	//Ramdrive
	EFI_PHYSICAL_ADDRESS RamDriveAddress;

	//PDB
	EFI_PHYSICAL_ADDRESS PdbAddress;
	size_t PdbSize;
} LOADER_PARAMS, * PLOADER_PARAMS;

//KernelMain
typedef void (*KernelMain)(LOADER_PARAMS* params);