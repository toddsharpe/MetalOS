#include "Device.h"

#include <efilib.h>
#include "EfiMain.h"
#include "Error.h"
#include <string.h>
#include "Output.h"

//TODO: move to file or link against GNU EFI lib
EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

// On Hyper-V, console/locate directly returns 1 GOP, whereas locating by handle returns 2. They all look the same
// Each GOP has one mode. So for now, take the 1 mode discovered via console

EFI_STATUS InitializeGraphics(PEFI_GRAPHICS_DEVICE pDevice)
{
	EFI_STATUS status;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE mode;

	//Get mode from UEFI
	ReturnIfNotSuccess(InitializeGopMode(&mode));

	//Populate kernel structure
	pDevice->FrameBufferBase = mode.FrameBufferBase;
	pDevice->FrameBufferSize = mode.FrameBufferSize;
	pDevice->HorizontalResolution = mode.Info->HorizontalResolution;
	pDevice->VerticalResolution = mode.Info->VerticalResolution;
	pDevice->PixelsPerScanLine = mode.Info->PixelsPerScanLine;

	return status;
}

//Originally this method was DiscoverGraphics and returned an array (multiple monitors), but lets not overcomplicate things.
//Initialize - pick a mode and return the info we need to operate in it
EFI_STATUS InitializeGopMode(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* display)
{
	EFI_STATUS status;

	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
	ReturnIfNotSuccess(BS->HandleProtocol(ST->ConsoleOutHandle, &GraphicsOutputProtocol, (void**)&gop));
	memcpy(display, gop->Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));

	//Allocate space for full graphics info
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info = nullptr;
	UINTN sizeOfInfo = 0;
	ReturnIfNotSuccess(gop->QueryMode(gop, 0, &sizeOfInfo, &info));
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, sizeOfInfo, (void**)&display->Info));
	memcpy(display->Info, info, sizeOfInfo);

	//Kernel will only support one pixel format. If we wait until the kernel is loaded then we cant print to give the error message.
	//We might be able to with different pixels, but lets just catch it here for now
	if (display->Info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor)
		return EFI_UNSUPPORTED;

	return EFI_SUCCESS;
}

//https://blog.fpmurphy.com/2015/05/check-available-text-and-graphics-modes-from-uefi-shell.html
EFI_STATUS DumpGopLocations()
{
	EFI_HANDLE* HandleBuffer = nullptr;
	UINTN HandleCount = 0;
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = nullptr;

	//Locate through Console
	Status = BS->HandleProtocol(ST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID**)&Gop);
	if (!EFI_ERROR(Status))
	{
		UartPrintf(L"GOP handle found via HandleProtocol\r\n");
		DumpGop(Gop);
	}
	else
		UartPrintf(L"No GOP handle found via HandleProtocol\r\n");

	//Locate directly
	Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
	if (!EFI_ERROR(Status) && Gop != NULL)
	{
		UartPrintf(L"Found GOP handle via LocateProtocol\r\n");
		DumpGop(Gop);
	}
	else
		UartPrintf(L"No GOP handle found via LocateProtocol\r\n");

	//Locate through handle
	Status = gBS->LocateHandleBuffer(ByProtocol,&gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleBuffer);
	if (!EFI_ERROR(Status))
	{
		UartPrintf(L"Found %d GOP handles via LocateHandleBuffer\r\n", HandleCount);
		for (int i = 0; i < HandleCount; i++)
		{
			Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiGraphicsOutputProtocolGuid, (VOID**)&Gop);
			if (!EFI_ERROR(Status))
				DumpGop(Gop);
			else
				UartPrintf(L"ERROR: Bad response from HandleProtocol: %d\r\n", Status);
		}
		gBS->FreePool(HandleBuffer);
	}
	else
		UartPrintf(L"No GOP handles found via LocateHandleBuffer\r\n");

	UartPrintf(L"\n");
	return Status;
}

EFI_STATUS DumpGop(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop)
{
	int i;
	int imax = gop->Mode->MaxMode;
	EFI_STATUS Status;

	UartPrintf(L"  GOP reports MaxMode %d\r\n", imax);

	for (i = 0; i < imax; i++)
	{
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
		UINTN SizeOfInfo;

		Status = gop->QueryMode(gop, i, &SizeOfInfo, &Info);
		if (EFI_ERROR(Status) && Status == EFI_NOT_STARTED)
		{
			gop->SetMode(gop, gop->Mode->Mode);
			Status = gop->QueryMode(gop, i, &SizeOfInfo, &Info);
		}

		if (EFI_ERROR(Status))
		{
			UartPrintf(L"    ERROR: Bad response from QueryMode: %d\r\n", Status);
			continue;
		}

		UartPrintf(L"    %c%d: %dx%d ", memcmp(Info, gop->Mode->Info, sizeof(*Info)) == 0 ? '*' : ' ', i,
			Info->HorizontalResolution,
			Info->VerticalResolution);
		switch (Info->PixelFormat)
		{
		case PixelRedGreenBlueReserved8BitPerColor:
			UartPrintf(L"RGBRerserved");
			break;
		case PixelBlueGreenRedReserved8BitPerColor:
			UartPrintf(L"BGRReserved");
			break;
		case PixelBitMask:
			UartPrintf(L"Red:%08x Green:%08x Blue:%08x Reserved:%08x",
				Info->PixelInformation.RedMask,
				Info->PixelInformation.GreenMask,
				Info->PixelInformation.BlueMask,
				Info->PixelInformation.ReservedMask);
			break;
		case PixelBltOnly:
			UartPrintf(L"(blt only)");
			break;
		default:
			UartPrintf(L"(Invalid pixel format)");
			break;
		}
		UartPrintf(L" Pixels %d\r\n", Info->PixelsPerScanLine);
	}

	return EFI_SUCCESS;
}
