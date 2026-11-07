#include "kernel/Kernel_display.h"
#include "kernel/Kernel.h"
#include "MetalOS.Loader.h"

KFramebufferInfo KeGetFramebuffer()
{
	return {
		BootParams.Display.FrameBufferBase,
		BootParams.Display.FrameBufferSize,
		BootParams.Display.HorizontalResolution,
		BootParams.Display.VerticalResolution,
		BootParams.Display.PixelsPerScanLine,
	};
}
