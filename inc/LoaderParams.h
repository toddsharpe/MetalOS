#pragma once

typedef struct
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Items;
	UINT32 Length;
} GPU_ARRAY;

typedef struct
{
	EFI_PHYSICAL_ADDRESS BaseAddress;
	EFI_RUNTIME_SERVICES Runtime;

	//Memory
	UINTN MemoryMapKey;
	UINTN MemoryMapDescriptorSize;
	UINT32 MemoryMapVersion;
	EFI_MEMORY_DESCRIPTOR* MemoryMap;

	//No point in using the array its always being used in a VM
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE Display;
} LOADER_PARAMS;

//KernelMain
typedef void (*KernelMain)(LOADER_PARAMS* params);
