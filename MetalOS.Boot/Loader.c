#include "Loader.h"
#include "Common.h"


EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle)
{
	EFI_STATUS status;
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)& LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	
	CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;
	Print(L"BootPath: %S\r\n", BootFilePath);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));

	EFI_FILE* CurrentDriveRoot;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));



	return status;
}