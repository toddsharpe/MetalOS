#pragma once

#include <efi.h>
#include <efilib.h>

namespace EfiDevice
{
	EFI_STATUS InitializeGraphics(EFI_GRAPHICS_DEVICE& device);
}
