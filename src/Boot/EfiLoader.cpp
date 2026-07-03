#pragma once

#include "Boot/EfiLoader.h"
#include "Lib/String.h"
#include "Assert.h"
#include "WinPE.h"

namespace EfiLoader
{
	typedef void (*CrtInitializer)();

	//Allocate file info and leave room for path
	alignas(EFI_FILE_INFO) static uint8_t infoBuffer[sizeof(EFI_FILE_INFO) + 256] = {};

	EFI_STATUS MapFile(EFI_FILE * file, const EFI_MEMORY_TYPE allocationType, EFI_PHYSICAL_ADDRESS& addressOut, size_t& sizeOut)
	{
		//Get file info into a stack buffer (fits EFI_FILE_INFO plus the filename)
		UINTN infoSize = sizeof(infoBuffer);
		EFI_FILE_INFO* fileInfo = (EFI_FILE_INFO*)infoBuffer;
		ReturnIfNotSuccess(file->GetInfo(file, &gEfiFileInfoGuid, &infoSize, (void*)fileInfo));
		sizeOut = fileInfo->FileSize;

		//Allocate space for file
		ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, allocationType, x64::SizeToPages(sizeOut), &addressOut));

		//Read file into memory
		ReturnIfNotSuccess(file->Read(file, &sizeOut, (void*)addressOut));

		return EFI_SUCCESS;
	}

	//This method should check the memory map file and ensure nobody else has this reservation
	EFI_STATUS MapKernel(EFI_FILE* pFile, UINT64& imageSizeOut, UINT64& entryPointOut, EFI_PHYSICAL_ADDRESS& physicalImageBaseOut)
	{
		EFI_STATUS status;

		//Dos header
		UINTN size = sizeof(IMAGE_DOS_HEADER);
		IMAGE_DOS_HEADER dosHeader;
		ReturnIfNotSuccess(pFile->Read(pFile, &size, &dosHeader));
		Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

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
		ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, x64::SizeToPages((UINTN)peHeader.OptionalHeader.SizeOfImage), &physicalImageBaseOut));

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
		bool relocate = pNtHeader->OptionalHeader.ImageBase != KernelBase;
		Assert(!relocate);
		//Update NTHeader to point to new virtual address
		pNtHeader->OptionalHeader.ImageBase = KernelBase;

		//Relocate image to KernelSpace. It gets allocated at KernelStart + ImageBase
		IMAGE_DATA_DIRECTORY relocationDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
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

		Print(L"  ImageBase: 0x%016x ImageSize: 0x%08x\r\n", KernelBase, imageSizeOut);
		Print(L"  Entry: 0x%016x Physical: 0x%016x\r\n", entryPointOut, physicalImageBaseOut);

		return EFI_SUCCESS;
	}

	//This function doesn't do any error checking, should it?
	EFI_STATUS CrtInitialization(const void* const imageBase)
	{
		//Find CRT section
		const IMAGE_SECTION_HEADER* crtSection = WinPE::GetPESection(imageBase, ".CRT");
		if (crtSection == nullptr)
			return EFI_NOT_FOUND;

		//https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-initialization?view=vs-2019
		//https://docs.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-warning-lnk4210?view=vs-2019
		//.CRT seems to be a list of function pointers (see asm). Loop through each one and invoke them
		CrtInitializer* initializer = (CrtInitializer*)((uintptr_t)imageBase + crtSection->VirtualAddress);
		while (*initializer)
		{
			(*initializer)();
			initializer++;
		}

		return EFI_SUCCESS;
	}

	void* GetProcAddress(void* const imageBase, const CString& procName)
	{
		//Headers
		PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		Assert(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size);

		PIMAGE_DATA_DIRECTORY directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		if ((directory->Size == 0) || (directory->VirtualAddress == 0))
			return NULL;

		PIMAGE_EXPORT_DIRECTORY exportDirectory = MakePointer<PIMAGE_EXPORT_DIRECTORY>(imageBase, directory->VirtualAddress);

		PDWORD pNames = MakePointer<PDWORD>(imageBase, exportDirectory->AddressOfNames);
		PWORD pOrdinals = MakePointer<PWORD>(imageBase, exportDirectory->AddressOfNameOrdinals);
		PDWORD pFunctions = MakePointer<PDWORD>(imageBase, exportDirectory->AddressOfFunctions);

		uintptr_t search = 0;
		for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++)
		{
			char * const name = MakePointer<char*>(imageBase, pNames[i]);
			if (procName == CString(name))
			{
				WORD ordinal = pOrdinals[i];
				search = MakePointer<uintptr_t>(imageBase, pFunctions[ordinal]);
			}
		}

		//Check if forwarded
		uintptr_t base = (uintptr_t)imageBase + directory->VirtualAddress;
		if ((search >= base) && (search < (base + directory->Size)))
		{
			return NULL;
		}

		//If function is forwarded, (PCHAR)search is its name
		//DWORD base = (DWORD)hModule + directory->VirtualAddress;
		//if ((search >= base) && (search < (base + directory->Size)))
		//{
		//    char* name = (char*)search;
		//    char* copy = (char*)malloc((strlen(name) + 1) * sizeof(char));
		//    strcpy(copy, name);
		//    char* library = strtok(copy, ".");
		//    char* function = strtok(NULL, ".");

		//    HMODULE hLibrary = LoadLibrary(library);//Get the address
		//    return GetExportAddress(hLibrary, function);
		//}

		return (void*)search;
	}
}
