#include "EfiLoader.h"

#include <efilib.h>
#include <kernel/MetalOS.Internal.h>
#include <kernel/LoaderParams.h>
#include <windows/types.h>
#include <windows/winnt.h>
#include "EfiMain.h"
#include "Error.h"
#include <string.h>

EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

EFI_GUID gEfiFileInfoGuid = EFI_FILE_INFO_ID;

typedef void (*CrtInitializer)();

EFI_STATUS EfiLoader::MapFile(EFI_FILE* file, EFI_PHYSICAL_ADDRESS& addressOut, size_t& sizeOut)
{
	EFI_STATUS status;
	
	//Get file info size
	UINTN infoSize = 0;
	status = file->GetInfo(file, &gEfiFileInfoGuid, &infoSize, nullptr);
	if (status != EFI_BUFFER_TOO_SMALL)
	{
		ReturnIfNotSuccess(status);
	}

	//Allocate space for file info
	EFI_FILE_INFO* fileInfo;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, infoSize, (void**)&fileInfo));

	//Get file info
	ReturnIfNotSuccess(file->GetInfo(file, &gEfiFileInfoGuid, &infoSize, (void*)fileInfo));
	sizeOut = fileInfo->FileSize;

	//Allocate space for file
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, AllocationType, EFI_SIZE_TO_PAGES(sizeOut), &addressOut));

	//Read file into memory
	ReturnIfNotSuccess(file->Read(file, &sizeOut, (void*)addressOut));

	return EFI_SUCCESS;
}

//This method should check the memory map file and ensure nobody else has this reservation
EFI_STATUS EfiLoader::MapKernel(EFI_FILE* pFile, UINT64& imageSizeOut, UINT64& entryPointOut, EFI_PHYSICAL_ADDRESS& physicalImageBaseOut)
{
	EFI_STATUS status;

	//Dos header
	UINTN size = sizeof(IMAGE_DOS_HEADER);
	IMAGE_DOS_HEADER dosHeader;
	ReturnIfNotSuccess(pFile->Read(pFile, &size, &dosHeader));
	ReturnIfNotSuccess(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	size = sizeof(IMAGE_NT_HEADERS64);
	IMAGE_NT_HEADERS64 peHeader;
	ReturnIfNotSuccess(pFile->SetPosition(pFile, (UINT64)dosHeader.e_lfanew));
	ReturnIfNotSuccess(pFile->Read(pFile, &size, &peHeader));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
		peHeader.OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_NATIVE)
		return EFI_UNSUPPORTED;

	//Kernel cant have imports yet
	if (peHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0)
		ReturnIfNotSuccess(EFI_UNSUPPORTED);

	//Allocate pages for full image
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, EFI_SIZE_TO_PAGES((UINTN)peHeader.OptionalHeader.SizeOfImage), &physicalImageBaseOut));

	//Read headers into memory
	size = peHeader.OptionalHeader.SizeOfHeaders;
	ReturnIfNotSuccess(pFile->SetPosition(pFile, 0));
	ReturnIfNotSuccess(pFile->Read(pFile, &size, (void*)physicalImageBaseOut));

	//Pointer into NTHeader loaded in memory
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)(physicalImageBaseOut + dosHeader.e_lfanew);
	
	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		EFI_PHYSICAL_ADDRESS destination = physicalImageBaseOut + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		UINTN rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			ReturnIfNotSuccess(pFile->SetPosition(pFile, section[i].PointerToRawData));
			ReturnIfNotSuccess(pFile->Read(pFile, &rawSize, (void*)destination));
		}
	}

	//TODO: Remove relocation logic, it has no meaning as we are allocating in Identity Paging
	bool relocate = pNtHeader->OptionalHeader.ImageBase != KernelBaseAddress;
	//Update NTHeader to point to new virtual address
	pNtHeader->OptionalHeader.ImageBase = KernelBaseAddress;

	//Relocate image to KernelSpace. It gets allocated at KernelStart + ImageBase
	IMAGE_DATA_DIRECTORY relocationDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	Print(L"  Relocations: %u Relocation Needed: %d\r\n", relocationDirectory.Size, relocate);
	if (relocationDirectory.Size)
	{
		PIMAGE_BASE_RELOCATION pBaseRelocation = (PIMAGE_BASE_RELOCATION)(physicalImageBaseOut + relocationDirectory.VirtualAddress);

		//Calculate relative shift by subtracing location live ImageBase from ImageBase first read from image
		UINT64 delta = (UINT64)pNtHeader->OptionalHeader.ImageBase - (UINT64)peHeader.OptionalHeader.ImageBase;

		while (pBaseRelocation->VirtualAddress)
		{
			PBYTE locationBase = (PBYTE)(physicalImageBaseOut + pBaseRelocation->VirtualAddress);
			PWORD locationData = (PWORD)((UINT64)pBaseRelocation + sizeof(IMAGE_BASE_RELOCATION));

			for (DWORD i = 0; i < (pBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD); i++, locationData++)
			{
				int type = (*locationData >> 12);
				int offset = (*locationData & 0x0FFF);

				switch (type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					*(DWORD*)(locationBase + offset) += (DWORD)delta;
					break;

				case IMAGE_REL_BASED_DIR64:
					*(UINT64*)(locationBase + offset) += delta;
					break;
				}
			}

			pBaseRelocation = (PIMAGE_BASE_RELOCATION)((UINT64)pBaseRelocation + pBaseRelocation->SizeOfBlock);
		}
	}

	//Populate return variables
	imageSizeOut = pNtHeader->OptionalHeader.SizeOfImage;
	entryPointOut = pNtHeader->OptionalHeader.ImageBase + pNtHeader->OptionalHeader.AddressOfEntryPoint;

	Print(L"  ImageBase: 0x%016x ImageSize: 0x%08x\r\n", KernelBaseAddress, imageSizeOut);
	Print(L"  Entry: 0x%016x Physical: 0x%016x\r\n", entryPointOut, physicalImageBaseOut);

	return EFI_SUCCESS;
}

//This function doesn't do any error checking, should it?
EFI_STATUS EfiLoader::CrtInitialization(const uintptr_t imageBase)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)imageBase;
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)(imageBase + dosHeader->e_lfanew);

	//Find CRT section
	PIMAGE_SECTION_HEADER crtSection = nullptr;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		if (strcmp((char*)& section[i].Name, ".CRT") == 0)
		{
			crtSection = &section[i];
			break;
		}
	}

	if (crtSection == nullptr)
		return EFI_NOT_FOUND;
	
	//https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-initialization?view=vs-2019
	//https://docs.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-warning-lnk4210?view=vs-2019
	//.CRT seems to be a list of function pointers (see asm). Loop through each one and invoke them
	CrtInitializer* initializer = (CrtInitializer*)(imageBase + crtSection->VirtualAddress);
	while (*initializer)
	{
		Print(L"Executing: 0x%016llx\n", *initializer);
		(*initializer)();
		initializer++;
	}

	return EFI_SUCCESS;
}
