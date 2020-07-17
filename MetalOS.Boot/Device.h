#pragma once

#include <efi.h>
#include <LoaderParams.h>

//Graphics
EFI_STATUS InitializeGraphics(PEFI_GRAPHICS_DEVICE pDevice);
EFI_STATUS InitializeGopMode(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display);

EFI_STATUS DumpGopLocations();
EFI_STATUS DumpGop(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop);
