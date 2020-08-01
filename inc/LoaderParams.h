#pragma once

#include <stdint.h>

//PixelsPerScanLine could be larger than HorizonalResolution if theres padding
typedef struct
{
	UINTN FrameBufferBase;
	UINT32 FrameBufferSize;
	UINT32 HorizontalResolution;
	UINT32 VerticalResolution;
	UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_DEVICE, * PEFI_GRAPHICS_DEVICE;

//Windows Internals, Part 2, Page 297
enum class PageState : uint8_t
{
	Zeroed,
	Free,
	//Standby,
	//Modified,
	//ModifiedNoWrite,
	//Bad,
	Active,
	//Transition,
	Platform //Used for acpi/runtime/reserved pages
};

//Windows Internals, Part 2, Page 316
typedef struct _PFN_ENTRY
{
	//Pointers when placing frames on the lists
	_PFN_ENTRY* Prev;
	_PFN_ENTRY* Next;

	//uintptr_t PTEAddress;
	//uint8_t ShareCount;
	PageState State;

	struct
	{
		uint8_t WriteInProgress : 1;
		uint8_t Modified : 1;
		uint8_t ReadInProgress : 1;
	} Flags;
	uint8_t Priority;//Unused

	uintptr_t PteFrame;

} PFN_ENTRY, * PPFN_ENTRY;

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