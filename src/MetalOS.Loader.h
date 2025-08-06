#pragma once

#include <efi.h>
#include "core_crt/stdint.h"
#include "Lib/System.h"
#include "Lib/List.h"

//PixelsPerScanLine could be larger than HorizonalResolution if theres padding
struct EFI_GRAPHICS_DEVICE
{
	paddr_t FrameBufferBase;
	size_t FrameBufferSize;
	uint32_t HorizontalResolution;
	uint32_t VerticalResolution;
	uint32_t PixelsPerScanLine;
};

struct EFI_MEMORY_MAP
{
	//MemoryMap - allocated on its own page
	EFI_MEMORY_DESCRIPTOR* Table;
	size_t Size;
	size_t DescriptorSize;
};

enum class PageState
{
	Free,
	Active,
	Reserved //Used for acpi/runtime/reserved pages
};

//Simple struct to represent a physical page. Its state and list pointers
struct PageFrame
{
	//Pointers when placing frames on the lists
	ListEntry Link;
	PageState State;
};

//No point in supporting multiple monitors since this is built for hyper-v
struct LoaderParams
{
	//Kernel
	paddr_t KernelAddress;
	size_t KernelImageSize;

	//UEFI Runtime
	EFI_RUNTIME_SERVICES* Runtime;

	//Memory map
	EFI_MEMORY_MAP MemoryMap;

	//Page Table Pool
	paddr_t PageTablesPoolAddress;
	uint32_t PageTablesPoolPageCount;

	//Page Frame DB
	paddr_t PageFrameAddr;
	size_t PageFrameCount;

	EFI_GRAPHICS_DEVICE Display;

	//Config tables
	EFI_CONFIGURATION_TABLE* ConfigTables;
	size_t ConfigTablesCount;

	//Ramdrive
	paddr_t RamDriveAddress;
	//PDB
	paddr_t PdbAddress;
	size_t PdbSize;
};

//Bootloader constants
static constexpr size_t RamDriveSize = Megabytes(64);

//KernelMain
typedef void (*KernelMain)();
