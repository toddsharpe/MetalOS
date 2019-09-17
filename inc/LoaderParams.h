#pragma once

typedef struct
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Items;//??? - EFI_GRAPHICS_OUTPUT_PROTOCOL
	UINT32 Length;
} GPU_ARRAY;

typedef struct
{
	EFI_PHYSICAL_ADDRESS BaseAddress;
	EFI_RUNTIME_SERVICES Runtime;

	SIMPLE_TEXT_OUTPUT_INTERFACE* ConOut;

	//No point in using the array its always being used in a VM
	EFI_GRAPHICS_OUTPUT_PROTOCOL Graphics;
} LOADER_PARAMS;