#include "Loader.h"
#include "Common.h"
#include "Path.h"
#include "WindowsPE.h"
#include "Memory.h"

EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

#define Kernel L"moskrnl.exe"

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle)
{
	EFI_STATUS status;
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)& LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	
	CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;
	Print(L"BootFilePath: %S\r\n", BootFilePath);

	CHAR16 KernelPath[64];
	PathCombine(BootFilePath, KernelPath, Kernel);
	Print(L"Loading: %S\r\n", KernelPath);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));

	EFI_FILE* CurrentDriveRoot;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));

	EFI_FILE* KernelFile;
	ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &KernelFile, KernelPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));

	EFI_PHYSICAL_ADDRESS imageBase = 0;
	EFI_PHYSICAL_ADDRESS entryPoint = 0;
	ReturnIfNotSuccess(MapFile(KernelFile, &imageBase, &entryPoint));

	return status;
}

//This method should check the memory map file and ensure nobody else has this reservation
EFI_STATUS MapFile(EFI_FILE* file, EFI_PHYSICAL_ADDRESS* imageBaseOut, EFI_PHYSICAL_ADDRESS* entryPoint)
{
	EFI_STATUS status;

	Print(L"MapFile\r\n");

	UINTN size = sizeof(IMAGE_DOS_HEADER);
	IMAGE_DOS_HEADER dosHeader;
	ReturnIfNotSuccess(file->Read(file, &size, &dosHeader));
	if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
		return EFI_UNSUPPORTED;

	ReturnIfNotSuccess(file->SetPosition(file, (UINT64)dosHeader.e_lfanew));

	size = sizeof(IMAGE_NT_HEADERS64);
	IMAGE_NT_HEADERS64 peHeader;
	ReturnIfNotSuccess(file->Read(file, &size, &peHeader));

	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		return EFI_UNSUPPORTED;

	if (peHeader.OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_NATIVE)
		return EFI_UNSUPPORTED;

	EFI_PHYSICAL_ADDRESS imageBase = peHeader.OptionalHeader.ImageBase;
	Print(L"ImageBase: %q\r\n", imageBase);

	//Verification is complete
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(peHeader.OptionalHeader.SizeOfHeaders), &imageBase));

	//Read headers into memory
	size = peHeader.OptionalHeader.SizeOfHeaders;
	ReturnIfNotSuccess(file->Read(file, &size, (void*)imageBase));

	if (imageBaseOut != NULL)
		*imageBaseOut = imageBase;

	//Write sections
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(&peHeader);
	Print(L"%q - %q - %w\r\n", &peHeader, section, peHeader.FileHeader.NumberOfSections);
	for (WORD i = 0; i < peHeader.FileHeader.NumberOfSections; i++)
	{
		EFI_PHYSICAL_ADDRESS destination = imageBase + section[i].VirtualAddress;
		Print(L"T: %u\r\n", section[i].VirtualAddress);
		EFI_PHYSICAL_ADDRESS source = section[i].PointerToRawData;
		DWORD size = section[i].SizeOfRawData;
		//ReturnIfNotSuccess(BS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(size), &destination));

	}

	return EFI_SUCCESS;
}