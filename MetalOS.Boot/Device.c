#include "Device.h"
#include "Common.h"
#include "Print.h"
#include "Memory.h"

//TODO: move to file?
EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

// Adapted from: https://blog.fpmurphy.com/2015/05/check-available-text-and-graphics-modes-from-uefi-shell.html
// On Hyper-V, console/locate directly returns 1 GOP, whereas locating by handle returns 2. They all look the same
// Each GOP has one mode. So for now, take the 1 mode discovered via console

//Originally this method was DiscoverGraphics and returned an array, but lets not overcomplicate things.
//Initialize - pick a mode and return the info we need to operate in it
EFI_STATUS InitializeGraphics(EFI_GRAPHICS_OUTPUT_PROTOCOL* graphics)
{
	EFI_STATUS status;

	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	ReturnIfNotSuccess(BS->HandleProtocol(ST->ConsoleOutHandle, &GraphicsOutputProtocol, (void**)&gop));

	//Copy to loader params
	efi_memcpy(gop, graphics, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL));

	//Allocate space for full graphics info
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
	UINTN sizeOfInfo;
	ReturnIfNotSuccess(gop->QueryMode(gop, 0, &sizeOfInfo, &info));
	ReturnIfNotSuccess(BS->AllocatePool(EfiLoaderData, sizeOfInfo, &graphics->Mode->Info));
	efi_memcpy(info, graphics->Mode->Info, sizeOfInfo);

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
}

EFI_STATUS
CheckGOP(BOOLEAN Verbose)
{
	EFI_HANDLE* HandleBuffer = NULL;
	UINTN HandleCount = 0;
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;


	// get from ConsoleOutHandle?
	Status = gBS->HandleProtocol(gST->ConsoleOutHandle,
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
				(VOID*)& Gop);
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
		Print(L"%c%d: %dx%d ", efi_memcmp(Info, gop->Mode->Info, sizeof(*Info)) == 0 ? '*' : ' ', i,
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

//==================================================================================================================================
//  print_memmap: The Ultimate Debugging Tool
//==================================================================================================================================
//
// Get the system memory map, parse it, and print it. Print the whole thing.
//

// This array is a global variable so that it can be made static, which helps prevent a stack overflow if it ever needs to lengthen.
STATIC CONST CHAR16 mem_types[16][27] = {
	  L"EfiReservedMemoryType     ",
	  L"EfiLoaderCode             ",
	  L"EfiLoaderData             ",
	  L"EfiBootServicesCode       ",
	  L"EfiBootServicesData       ",
	  L"EfiRuntimeServicesCode    ",
	  L"EfiRuntimeServicesData    ",
	  L"EfiConventionalMemory     ",
	  L"EfiUnusableMemory         ",
	  L"EfiACPIReclaimMemory      ",
	  L"EfiACPIMemoryNVS          ",
	  L"EfiMemoryMappedIO         ",
	  L"EfiMemoryMappedIOPortSpace",
	  L"EfiPalCode                ",
	  L"EfiPersistentMemory       ",
	  L"EfiMaxMemoryType          "
};

VOID print_memmap()
{
	EFI_STATUS memmap_status;
	UINTN MemMapSize = 0, MemMapKey, MemMapDescriptorSize;
	UINT32 MemMapDescriptorVersion;
	EFI_MEMORY_DESCRIPTOR* MemMap = NULL;
	EFI_MEMORY_DESCRIPTOR* Piece;
	UINT16 line = 0;

	memmap_status = BS->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
	if (memmap_status == EFI_BUFFER_TOO_SMALL)
	{
		memmap_status = BS->AllocatePool(EfiBootServicesData, MemMapSize, (void**)& MemMap); // Allocate pool for MemMap
		if (EFI_ERROR(memmap_status)) // Error! Wouldn't be safe to continue.
		{
			Print(L"MemMap AllocatePool error. 0x%q\r\n", memmap_status);
			return;
		}
		memmap_status = BS->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
	}
	if (EFI_ERROR(memmap_status))
	{
		Print(L"Error getting memory map for printing. 0x%q\r\n", memmap_status);
	}

	Print(L"MemMapSize: %q, MemMapDescriptorSize: %q, MemMapDescriptorVersion: 0x%q\r\n", MemMapSize, MemMapDescriptorSize, MemMapDescriptorVersion);
	Print(L"MemMap: %q\r\n", MemMap);

	// There's no virtual addressing yet, so there's no need to see Piece->VirtualStart
	// Multiply NumOfPages by EFI_PAGE_SIZE or do (NumOfPages << EFI_PAGE_SHIFT) to get the end address... which should just be the start of the next section.
	for (Piece = MemMap; Piece < (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemMap + MemMapSize); Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + MemMapDescriptorSize))
	{
		if (line % 20 == 0)
		{
			Keywait(L"\0");
			Print(L"#   Memory Type                Phys Addr Start   Num Of Pages\r\n");
		}

		Print(L"%w: %S 0x%q 0x%q\r\n", line, mem_types[Piece->Type], Piece->PhysicalStart, Piece->NumberOfPages);
		line++;
	}

	memmap_status = BS->FreePool(MemMap);
	if (EFI_ERROR(memmap_status))
	{
		Print(L"Error freeing print_memmap pool. 0x%d\n", memmap_status);
	}
}