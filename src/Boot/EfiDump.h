#pragma once

#include <Efi.h>
#include "MetalOS.Loader.h"

namespace EfiDump
{
	EFI_STATUS DumpGop(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop);

	//TODO: rewrite to dump to uart
	EFI_STATUS DumpAllProtocols(EFI_HANDLE ImageHandle);

	//https://blog.fpmurphy.com/2015/05/check-available-text-and-graphics-modes-from-uefi-shell.html
	EFI_STATUS DumpGopLocations();

	EFI_STATUS PrintDirectory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir);
}
