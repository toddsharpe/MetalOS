#pragma once

#include "boot/EfiDump.h"

#define File L"dump.txt"
#define MaxFilePath 64

namespace EfiDump
{
	EFI_STATUS DumpGop(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop)
	{
		EFI_STATUS Status;

		UartPrintf(L"  GOP reports MaxMode %d\r\n", gop->Mode->MaxMode);

		for (uint32_t i = 0; i < gop->Mode->MaxMode; i++)
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
	
	//TODO: rewrite to dump to uart
	EFI_STATUS DumpAllProtocols(EFI_HANDLE ImageHandle)
	{
		EFI_STATUS status;

		//Access this image
		EFI_LOADED_IMAGE* LoadedImage = nullptr;
		ReturnIfNotSuccess(BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
		CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;

		//Build output path
		CHAR16* FullPath = nullptr;
		ReturnIfNotSuccess(BS->AllocatePool(LoadedImage->ImageDataType, MaxFilePath * sizeof(CHAR16), (void**)&FullPath));
		memset(FullPath, 0, MaxFilePath * sizeof(CHAR16));
		GetDirectoryName(BootFilePath, FullPath);
		wcscpy(FullPath + wcslen(FullPath), File);

		//Get output file
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem = nullptr;
		ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
		EFI_FILE* CurrentDriveRoot = nullptr;
		ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));
		EFI_FILE* outputFile = nullptr;
		ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &outputFile, FullPath, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0));

		//Get all handles
		UINTN HandleCount;
		EFI_HANDLE* pHandleBuffer;
		ReturnIfNotSuccess(BS->LocateHandleBuffer(AllHandles, NULL, NULL, &HandleCount, &pHandleBuffer));

		//Buffer
		UINTN bufferSize;
		constexpr size_t count = 255;
		CHAR16 writeBuffer[count] = {};
		swprintf(writeBuffer, count, L"HandleCount %lld\r\n", HandleCount);
		bufferSize = wcslen(writeBuffer) * 2;
		ReturnIfNotSuccess(outputFile->Write(outputFile, &bufferSize, writeBuffer));

		UINTN ProtocolCount;
		EFI_GUID** pProtocolBuffer;
		UINTN ProtocolIndex;
		for (size_t i = 0; i < HandleCount; i++)
		{
			ReturnIfNotSuccess(BS->ProtocolsPerHandle(pHandleBuffer[i], &pProtocolBuffer, &ProtocolCount));

			for (ProtocolIndex = 0; ProtocolIndex < ProtocolCount; ProtocolIndex++)
			{
				EFI_GUID* guid = pProtocolBuffer[ProtocolIndex];
				if (0 == ProtocolIndex)
				{
					swprintf(writeBuffer, count, L"Handle 0x%016llx (%lld):\r\n    {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\r\n",
						(uintptr_t)pHandleBuffer[i],
						ProtocolCount,
						guid->Data1,
						guid->Data2, guid->Data3,
						guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
						guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
					bufferSize = wcslen(writeBuffer) * 2;
					ReturnIfNotSuccess(outputFile->Write(outputFile, &bufferSize, writeBuffer));
				}
				else
				{
					swprintf(writeBuffer, count, L"    {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\r\n",
						guid->Data1,
						guid->Data2, guid->Data3,
						guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
						guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
					bufferSize = wcslen(writeBuffer) * 2;
					ReturnIfNotSuccess(outputFile->Write(outputFile, &bufferSize, writeBuffer));
				}
			}

			BS->FreePool(pProtocolBuffer);
		}

		BS->FreePool(pHandleBuffer);

		outputFile->Close(outputFile);
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
		Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleBuffer);
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

	EFI_STATUS DumpMemoryMap(const EFI_MEMORY_MAP& map)
	{
		const CHAR16 mem_types[16][27] = {
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

		UartPrintf(L"MapSize: 0x%016X, Size: 0x%x, DescSize: 0x%x\n", map.Table, map.Size, map.DescriptorSize);

		for (EFI_MEMORY_DESCRIPTOR* current = map.Table; current < NextMemoryDescriptor(map.Table, map.Size); current = NextMemoryDescriptor(current, map.DescriptorSize))
		{
			const bool runtime = (current->Attribute & EFI_MEMORY_RUNTIME) != 0;
			UartPrintf(L"P: %016x V: %016x T:%s #: 0x%x A:0x%016x %c\n", current->PhysicalStart, current->VirtualStart, mem_types[current->Type], current->NumberOfPages, current->Attribute, runtime ? 'R' : ' ');
		}

		return EFI_SUCCESS;
	}

	EFI_STATUS PrintDirectory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir)
	{
		EFI_STATUS status;

		const size_t MAX_FILE_INFO_SIZE = 1024;
		uint8_t buffer[MAX_FILE_INFO_SIZE] = { };
		EFI_FILE_INFO* fileInfo = (EFI_FILE_INFO*)buffer;

		while (true)
		{
			UINTN size = MAX_FILE_INFO_SIZE;
			ReturnIfNotSuccess(status = dir->Read(dir, &size, buffer));
			if (size == 0)
				break; //No more directories

			if (wcscmp(fileInfo->FileName, L".") == 0 || wcscmp(fileInfo->FileName, L"..") == 0)
			{
				continue;
			}

			if (fileInfo->Attribute & EFI_FILE_DIRECTORY)
			{
				UartPrintf(L"Dir: %s\n", fileInfo->FileName);

				//recurse
				EFI_FILE_HANDLE subDir;
				ReturnIfNotSuccess(dir->Open(dir, &subDir, fileInfo->FileName, EFI_FILE_MODE_READ, 0));
				subDir->SetPosition(subDir, 0);

				PrintDirectory(fs, subDir);

				dir->Close(subDir);
			}
			else
				UartPrintf(L"File: %s\n", fileInfo->FileName);
		}

		return status;
	}
}
