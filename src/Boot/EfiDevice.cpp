#include "Boot/EfiDevice.h"

namespace EfiDevice
{
	// On Hyper-V, console/locate directly returns 1 GOP, whereas locating by handle returns 2. They all look the same
	// Each GOP has one mode. So for now, take the 1 mode discovered via console
	EFI_STATUS InitializeGraphics(EFI_GRAPHICS_DEVICE& device)
	{
		//Get current mode
		EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
		ReturnIfNotSuccess(BS->HandleProtocol(ST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (void**)&gop));

		//Allocate space for full graphics info
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info = nullptr;
		UINTN size = 0;
		ReturnIfNotSuccess(gop->QueryMode(gop, gop->Mode->Mode, &size, &info));

		//Kernel will only support one pixel format
		if (info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor)
			return EFI_UNSUPPORTED;

		//Populate kernel structure
		device.FrameBufferBase = gop->Mode->FrameBufferBase;
		device.FrameBufferSize = gop->Mode->FrameBufferSize;
		device.HorizontalResolution = info->HorizontalResolution;
		device.VerticalResolution = info->VerticalResolution;
		device.PixelsPerScanLine = info->PixelsPerScanLine;

		return EFI_SUCCESS;
	}
}
