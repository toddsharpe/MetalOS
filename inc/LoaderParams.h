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

//Address Space Details
#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

//4mb reserved space
#define BootloaderPagePoolCount 32
#define ReservedPageTablePages 512
#define ReservedPageTableSpace (ReservedPageTablePages * EFI_PAGE_SIZE)
#define ReservedPageTableSpaceMask (ReservedPageTableSpace - 1)

//User space starts at   0x00000000 00000000
//User space stops at    0x00007FFF FFFFFFFF
//Kernel space starts at 0xFFFF8000 00000000
//Kernel space stops at  0xFFFFFFFF FFFFFFFF

#define UserStop    0x00007FFFFFFFFFFF
#define KernelStart 0xFFFF800000000000
#define KernelStop UINT64_MAX

#define PAGE_SIZE (1 << 12)

#define PAGE_SIZE   4096
#define PAGE_MASK   0xFFF
#define PAGE_SHIFT  12

#define SIZE_TO_PAGES(a) (((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

