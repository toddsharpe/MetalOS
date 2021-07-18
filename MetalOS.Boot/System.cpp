#include "System.h"

#include <crt_string.h>
#include <efilib.h>
#include "crt_string.h"
#include "crt_wchar.h"
#include <Path.h>
#include "Error.h"
#include "EfiMain.h"

#define File L"dump.txt"
#define MaxFilePath 64

//TODO: rewrite to dump to uart
EFI_STATUS DumpAllProtocols(EFI_HANDLE ImageHandle)
{
	EFI_STATUS status;
	
	//Access this image
	EFI_LOADED_IMAGE* LoadedImage;
	ReturnIfNotSuccess(BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)& LoadedImage, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	AllocationType = LoadedImage->ImageDataType;
	CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;

	//Build output path
	CHAR16* FullPath;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, MaxFilePath * sizeof(CHAR16), (void**)&FullPath));
	memset((void*)FullPath, 0, MaxFilePath * sizeof(CHAR16));
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
	CHAR16 writeBuffer[255];
	memset(writeBuffer, 0, sizeof(writeBuffer));

	swprintf(writeBuffer, L"HandleCount %d\r\n", HandleCount);
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
				swprintf(writeBuffer, L"Handle 0x%016x (%d):\r\n    {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\r\n",
					pHandleBuffer[i], ProtocolCount,
					guid->Data1,
					guid->Data2, guid->Data3,
					guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
					guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
				bufferSize = wcslen(writeBuffer) * 2;
				ReturnIfNotSuccess(outputFile->Write(outputFile, &bufferSize, writeBuffer));
			}
			else
			{
				swprintf(writeBuffer, L"    {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\r\n",
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

