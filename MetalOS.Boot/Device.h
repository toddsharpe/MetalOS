#pragma once

#include <efi.h>
#include <efilib.h>

#include "LoaderParams.h"

EFI_STATUS InitializeGraphics(EFI_GRAPHICS_OUTPUT_PROTOCOL* graphics);
EFI_STATUS PrintGOP(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop);

// https://blog.fpmurphy.com/2015/05/check-available-text-and-graphics-modes-from-uefi-shell.html
EFI_STATUS CheckGOP(BOOLEAN Verbose);
EFI_STATUS PrintGOPFull(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop);
