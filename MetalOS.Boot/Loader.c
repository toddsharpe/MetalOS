#include "Loader.h"
#include "Common.h"
#include "Path.h"
#include "WindowsPE.h"
#include "Memory.h"
#include "Error.h"

EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

//This method should check the memory map file and ensure nobody else has this reservation
EFI_STATUS MapFile(EFI_FILE* file, EFI_PHYSICAL_ADDRESS* imageBaseOut, EFI_PHYSICAL_ADDRESS* entryPoint)
{
	EFI_STATUS status;

	//Print(L"MapFile\r\n");

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
	Print(L"  ImageBase: %q\r\n", imageBase);

	//Verification is complete
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES((UINTN)peHeader.OptionalHeader.SizeOfHeaders), &imageBase));

	//Read headers into memory
	size = peHeader.OptionalHeader.SizeOfHeaders;
	ReturnIfNotSuccess(file->SetPosition(file, 0));
	ReturnIfNotSuccess(file->Read(file, &size, (void*)imageBase));

	//Hack? - determine if loading headers on stack is right, versus just loading them from disk to physical memory
	//Problem is knowing the destination address
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)(imageBase + dosHeader.e_lfanew);
	//Print(L"  Subsystem: %u\r\n", pNtHeader->OptionalHeader.Subsystem);
	
	if (imageBaseOut != NULL)
		*imageBaseOut = imageBase;

	if (entryPoint != NULL)
		*entryPoint = imageBase + pNtHeader->OptionalHeader.AddressOfEntryPoint;

	//Write sections
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	//Print(L"  %q - %q - %w\r\n", pNtHeader, section, pNtHeader->FileHeader.NumberOfSections);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		EFI_PHYSICAL_ADDRESS destination = imageBase + section[i].VirtualAddress;
		//Print(L"  T: %s - %u\r\n", section[i].Name, section[i].VirtualAddress);
		//Print(L"    %u (%u) -> %u (%u)\r\n", section[i].PointerToRawData, section[i].SizeOfRawData,
		//	section[i].VirtualAddress, section[i].Misc.VirtualSize);

		//Allocate space according to virtual size
		ReturnIfNotSuccess(BS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES((UINTN)section[i].Misc.VirtualSize), &destination));

		//If physical size is non-zero, read data to allocated address
		UINTN rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			ReturnIfNotSuccess(file->SetPosition(file, section[i].PointerToRawData));
			ReturnIfNotSuccess(file->Read(file, &rawSize, (void*)destination));
		}
	}

	//Someday we could do imports but not now
	IMAGE_DATA_DIRECTORY importDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	//Print(L"  Imports: %u\r\n", importDirectory.Size);

	//TODO: relocations
	IMAGE_DATA_DIRECTORY relocationDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	//Print(L"  Relocations: %u\r\n", relocationDirectory.Size);

	//Print(L"  Base: %q Entry: %q\r\n", imageBase, (imageBase + pNtHeader->OptionalHeader.AddressOfEntryPoint));

	return EFI_SUCCESS;
}