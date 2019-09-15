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

	GPU_ARRAY GPUs;
} LOADER_PARAMS;