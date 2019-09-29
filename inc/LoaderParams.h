#pragma once

typedef struct
{
	EFI_PHYSICAL_ADDRESS BaseAddress;
	EFI_RUNTIME_SERVICES Runtime;

	//Memory
	EFI_MEMORY_DESCRIPTOR* MemoryMap;
	UINTN MemoryMapSize;
	UINTN MemoryMapDescriptorSize;
	UINT32 MemoryMapVersion;

	//No point in using the array its always being used in a VM
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE Display;

	//Config tables
	EFI_CONFIGURATION_TABLE* ConfigTables;
	UINTN ConfigTableSizes;
} LOADER_PARAMS;

//KernelMain
typedef void (*KernelMain)(LOADER_PARAMS* params);
