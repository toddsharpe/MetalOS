#pragma once

#include <efi.h>
#include <efilib.h>

#include "MetalOS.Internal.h"
#include <LoaderParams.h>

//Graphics
EFI_STATUS InitializeGraphics(PEFI_GRAPHICS_DEVICE pDevice);
EFI_STATUS PrintGraphicsDevice(PEFI_GRAPHICS_DEVICE pDevice);

EFI_STATUS PrintGOP(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop);
EFI_STATUS PrintGopMode(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* mode);

// https://blog.fpmurphy.com/2015/05/check-available-text-and-graphics-modes-from-uefi-shell.html
EFI_STATUS CheckGOP(BOOLEAN Verbose);
EFI_STATUS PrintGOPFull(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop);
