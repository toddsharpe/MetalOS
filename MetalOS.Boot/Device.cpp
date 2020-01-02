#include "Device.h"
#include "BootLoader.h"
#include "EfiPrint.h"
#include "Memory.h"
#include "Error.h"
#include "CRT.h"

//TODO: move to file?
EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

// Adapted from: https://blog.fpmurphy.com/2015/05/check-available-text-and-graphics-modes-from-uefi-shell.html
// On Hyper-V, console/locate directly returns 1 GOP, whereas locating by handle returns 2. They all look the same
// Each GOP has one mode. So for now, take the 1 mode discovered via console

//Originally this method was DiscoverGraphics and returned an array, but lets not overcomplicate things.
//Initialize - pick a mode and return the info we need to operate in it
EFI_STATUS InitializeGraphics(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display)
{
	EFI_STATUS status;

	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
	ReturnIfNotSuccess(BS->HandleProtocol(ST->ConsoleOutHandle, &GraphicsOutputProtocol, (void**)&gop));
	crt::memcpy(display, gop->Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));

	//Allocate space for full graphics info
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info = nullptr;
	UINTN sizeOfInfo = 0;
	ReturnIfNotSuccess(gop->QueryMode(gop, 0, &sizeOfInfo, &info));
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, sizeOfInfo, (void**)&display->Info));
	crt::memcpy(display->Info, info, sizeOfInfo);

	//Kernel will only support one pixel format. If we wait until the kernel is loaded then we cant print to give the error message.
	//We might be able to with different pixels, but lets just catch it here for now

	if (display->Info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor)
		return EFI_UNSUPPORTED;

	return EFI_SUCCESS;
}

EFI_STATUS InitializeGraphics(PGRAPHICS_DEVICE pDevice)
{
	EFI_STATUS status;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE mode;

	//Get mode from UEFI
	ReturnIfNotSuccess(InitializeGraphics(&mode));

	//Populate kernel structure
	pDevice->FrameBufferBase = mode.FrameBufferBase;
	pDevice->FrameBufferSize = mode.FrameBufferSize;
	pDevice->HorizontalResolution = mode.Info->HorizontalResolution;
	pDevice->VerticalResolution = mode.Info->VerticalResolution;
	pDevice->PixelsPerScanLine = mode.Info->PixelsPerScanLine;

	return status;
}

EFI_STATUS PrintGraphicsDevice(PGRAPHICS_DEVICE pDevice)
{
	EFI_STATUS status;
	
	ReturnIfNotSuccess(Print(L"Graphics:\r\n"));
	ReturnIfNotSuccess(Print(L"  FrameBuffer-Base %q, Size: %u\r\n", pDevice->FrameBufferBase, pDevice->FrameBufferSize));
	ReturnIfNotSuccess(Print(L"  Resulution 0x%4x (0x%4x) x 0x%4x\r\n", pDevice->HorizontalResolution, pDevice->PixelsPerScanLine, pDevice->VerticalResolution));

	return status;
}

EFI_STATUS PrintGopMode(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* mode)
{
	Print(L"SizeOfInfo %u\r\n", mode->SizeOfInfo);
	Print(L"FrameBufferSize %u\r\n", mode->FrameBufferSize);
	Print(L"FrameBufferBase %u\r\n", mode->FrameBufferBase);
	Print(L"&Res %dx%d - ", mode->Info->HorizontalResolution, mode->Info->VerticalResolution);

	switch (mode->Info->PixelFormat)
	{
	case PixelRedGreenBlueReserved8BitPerColor:
		Print(L"RGBRerserved");
		break;
	case PixelBlueGreenRedReserved8BitPerColor:
		Print(L"BGRReserved");
		break;
	case PixelBitMask:
		Print(L"Red:%08x Green:%08x Blue:%08x Reserved:%08x",
			mode->Info->PixelInformation.RedMask,
			mode->Info->PixelInformation.GreenMask,
			mode->Info->PixelInformation.BlueMask,
			mode->Info->PixelInformation.ReservedMask);
		break;
	case PixelBltOnly:
		Print(L"(blt only)");
		break;
	default:
		Print(L"(Invalid pixel format)");
		break;
	}
	Print(L" Pixels %d\n\r", mode->Info->PixelsPerScanLine);

	return EFI_SUCCESS;
}

EFI_STATUS PrintGOP(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop)
{
	Print(L"SizeOfInfo %u\r\n", gop->Mode->SizeOfInfo);
	Print(L"FrameBufferSize %u\r\n", gop->Mode->FrameBufferSize);
	Print(L"FrameBufferBase %u\r\n", gop->Mode->FrameBufferBase);
	Print(L"&Res %dx%d - ", gop->Mode->Info->HorizontalResolution, gop->Mode->Info->VerticalResolution);

	switch (gop->Mode->Info->PixelFormat)
	{
		case PixelRedGreenBlueReserved8BitPerColor:
			Print(L"RGBRerserved");
			break;
		case PixelBlueGreenRedReserved8BitPerColor:
			Print(L"BGRReserved");
			break;
		case PixelBitMask:
			Print(L"Red:%08x Green:%08x Blue:%08x Reserved:%08x",
				gop->Mode->Info->PixelInformation.RedMask,
				gop->Mode->Info->PixelInformation.GreenMask,
				gop->Mode->Info->PixelInformation.BlueMask,
				gop->Mode->Info->PixelInformation.ReservedMask);
			break;
		case PixelBltOnly:
			Print(L"(blt only)");
			break;
		default:
			Print(L"(Invalid pixel format)");
			break;
	}
	Print(L" Pixels %d\n\r", gop->Mode->Info->PixelsPerScanLine);

	return EFI_SUCCESS;
}

EFI_STATUS
CheckGOP(BOOLEAN Verbose)
{
	EFI_HANDLE* HandleBuffer = nullptr;
	UINTN HandleCount = 0;
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;

	// get from ConsoleOutHandle?
	Status = BS->HandleProtocol(ST->ConsoleOutHandle,
		&gEfiGraphicsOutputProtocolGuid,
		(VOID * *)& Gop);
	if (EFI_ERROR(Status)) {
		Print(L"No GOP handle found via HandleProtocol\r\n");
	}
	else {
		Print(L"GOP handle found via HandleProtocol\r\n");
		if (Verbose)
			PrintGOPFull(Gop);
	}

	// try locating directly
	Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
		NULL,
		(VOID * *)& Gop);
	if (EFI_ERROR(Status) || Gop == NULL) {
		Print(L"No GOP handle found via LocateProtocol\r\n");
	}
	else {
		Print(L"Found GOP handle via LocateProtocol\r\n");
		if (Verbose)
			PrintGOPFull(Gop);
	}

	// try locating by handle
	Status = gBS->LocateHandleBuffer(ByProtocol,
		&gEfiGraphicsOutputProtocolGuid,
		NULL,
		&HandleCount,
		&HandleBuffer);
	if (EFI_ERROR(Status)) {
		Print(L"No GOP handles found via LocateHandleBuffer\r\n");
	}
	else {
		Print(L"Found %d GOP handles via LocateHandleBuffer\r\n", HandleCount);
		for (int i = 0; i < HandleCount; i++) {
			Status = gBS->HandleProtocol(HandleBuffer[i],
				&gEfiGraphicsOutputProtocolGuid,
				(VOID**)& Gop);
			if (!EFI_ERROR(Status)) {
				if (Verbose)
					PrintGOPFull(Gop);
			}
		}
		//FreePool(HandleBuffer); - Linker error, who cares
	}

	Print(L"\n");

	return Status;
}

EFI_STATUS
PrintGOPFull(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop)
{
	int i, imax;
	EFI_STATUS Status;

	imax = gop->Mode->MaxMode;

	Print(L"GOP reports MaxMode %d\r\n", imax);

	for (i = 0; i < imax; i++) {
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
		UINTN SizeOfInfo;

		Status = gop->QueryMode(gop, i, &SizeOfInfo, &Info);
		if (EFI_ERROR(Status) && Status == EFI_NOT_STARTED) {
			gop->SetMode(gop, gop->Mode->Mode);
			Status = gop->QueryMode(gop, i, &SizeOfInfo, &Info);
		}

		if (EFI_ERROR(Status)) {
			Print(L"ERROR: Bad response from QueryMode: %d\r\n", Status);
			continue;
		}
		Print(L"%c%d: %dx%d ", crt::memcmp(Info, gop->Mode->Info, sizeof(*Info)) == 0 ? '*' : ' ', i,
			Info->HorizontalResolution,
			Info->VerticalResolution);
		switch (Info->PixelFormat) {
		case PixelRedGreenBlueReserved8BitPerColor:
			Print(L"RGBRerserved");
			break;
		case PixelBlueGreenRedReserved8BitPerColor:
			Print(L"BGRReserved");
			break;
		case PixelBitMask:
			Print(L"Red:%08x Green:%08x Blue:%08x Reserved:%08x",
				Info->PixelInformation.RedMask,
				Info->PixelInformation.GreenMask,
				Info->PixelInformation.BlueMask,
				Info->PixelInformation.ReservedMask);
			break;
		case PixelBltOnly:
			Print(L"(blt only)");
			break;
		default:
			Print(L"(Invalid pixel format)");
			break;
		}
		Print(L" Pixels %d\r\n", Info->PixelsPerScanLine);
	}
	Print(L"\r\n");

	return EFI_SUCCESS;
}
