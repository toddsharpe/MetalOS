#include "Loader.h"

//#include <crt_string.h>
#include <WindowsPE.h>
#include "Main.h"

bool Loader::ReadHeaders(const char* path, uintptr_t& imageBase, DWORD& imageSize)
{
	FileHandle* file = kernel.CreateFile(std::string(path), GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(kernel.ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	Assert(kernel.SetFilePosition(file, dosHeader.e_lfanew));
	Assert(kernel.ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		Assert(false);
	}

	imageBase = peHeader.OptionalHeader.ImageBase;
	imageSize = peHeader.OptionalHeader.SizeOfImage;

	return true;
}

Handle Loader::LoadKernelLibrary(const char* path)
{
	FileHandle* file = kernel.CreateFile(std::string(path), GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(kernel.ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	Assert(kernel.SetFilePosition(file, dosHeader.e_lfanew));
	Assert(kernel.ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		Assert(false);
		return nullptr;
	}

	void* address = kernel.AllocateKernelPage(peHeader.OptionalHeader.ImageBase, SIZE_TO_PAGES(peHeader.OptionalHeader.SizeOfImage), MemoryProtection::PageReadWriteExecute);
	if (address == nullptr)
	{
		address = kernel.AllocateKernelPage(0, SIZE_TO_PAGES(peHeader.OptionalHeader.SizeOfImage), MemoryProtection::PageReadWriteExecute);
	}
	Assert(address);

	//Read headers
	Assert(kernel.SetFilePosition(file, 0));
	Assert(kernel.ReadFile(file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	Assert(read == peHeader.OptionalHeader.SizeOfHeaders);

	PIMAGE_NT_HEADERS64 pNtHeader = MakePtr(PIMAGE_NT_HEADERS64, address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)address + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			Assert(kernel.SetFilePosition(file, section[i].PointerToRawData));
			Assert(kernel.ReadFile(file, (void*)destination, rawSize, &read));
			Assert(read == section[i].SizeOfRawData);
		}
	}

	//Relocations
	IMAGE_DATA_DIRECTORY relocationDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if (((uintptr_t)address != pNtHeader->OptionalHeader.ImageBase) && (relocationDirectory.Size))
	{
		PIMAGE_BASE_RELOCATION pBaseRelocation = MakePtr(PIMAGE_BASE_RELOCATION, address, relocationDirectory.VirtualAddress);

		//Calculate relative shift
		uintptr_t delta = (uintptr_t)address - (uintptr_t)pNtHeader->OptionalHeader.ImageBase;

		DWORD bytes = 0;
		while (bytes < relocationDirectory.Size)
		{
			PDWORD locationBase = MakePtr(PDWORD, address, pBaseRelocation->VirtualAddress);
			PWORD locationData = MakePtr(PWORD, pBaseRelocation, sizeof(IMAGE_BASE_RELOCATION));

			DWORD count = (pBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			for (DWORD i = 0; i < count; i++)
			{
				int type = (*locationData >> 12);
				int value = (*locationData & 0x0FFF);

				switch (type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					*MakePtr(PDWORD, locationBase, value) += delta;
					break;
				}

				locationData++;
			}

			bytes += pBaseRelocation->SizeOfBlock;
			pBaseRelocation = (PIMAGE_BASE_RELOCATION)locationData;
		}
	}

	//Kernel library dont have imports yet
	Assert(pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0);

	return address;
}

//TODO: this is very similar to KernelAPI's loader, as well as create process
Handle Loader::LoadLibrary(UserProcess& process, const char* path)
{
	FileHandle* file = kernel.CreateFile(std::string(path), GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(kernel.ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	Assert(kernel.SetFilePosition(file, dosHeader.e_lfanew));
	Assert(kernel.ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		Assert(false);
		return nullptr;
	}

	void* address = kernel.VirtualAlloc(process, (void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWriteExecute);
	Assert(address);
	
	//Read headers
	Assert(kernel.SetFilePosition(file, 0));
	Assert(kernel.ReadFile(file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	Assert(read == peHeader.OptionalHeader.SizeOfHeaders);

	Assert((uintptr_t)address == peHeader.OptionalHeader.ImageBase);

	PIMAGE_NT_HEADERS64 pNtHeader = MakePtr(PIMAGE_NT_HEADERS64, address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)address + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			Assert(kernel.SetFilePosition(file, section[i].PointerToRawData));
			Assert(kernel.ReadFile(file, (void*)destination, rawSize, &read));
			Assert(read == section[i].SizeOfRawData);
		}
	}

	//Imports are loaded from usermode, this is just for base kernelapi which shouldnt have any
	Assert(pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0);

	//Add to process's load map
	process.AddModule(path, address);

	return address;
}

uintptr_t Loader::GetProcAddress(Handle hModule, const char* lpProcName)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = MakePtr(PIMAGE_DOS_HEADER, hModule, 0);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	PIMAGE_NT_HEADERS64 ntHeader = MakePtr(PIMAGE_NT_HEADERS64, hModule, dosHeader->e_lfanew);
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	if (ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		return NULL;

	PIMAGE_DATA_DIRECTORY directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if ((directory->Size == 0) || (directory->VirtualAddress == 0))
		return NULL;

	PIMAGE_EXPORT_DIRECTORY exportDirectory = MakePtr(PIMAGE_EXPORT_DIRECTORY, hModule, directory->VirtualAddress);

	PDWORD pNames = MakePtr(PDWORD, hModule, exportDirectory->AddressOfNames);
	PWORD pOrdinals = MakePtr(PWORD, hModule, exportDirectory->AddressOfNameOrdinals);
	PDWORD pFunctions = MakePtr(PDWORD, hModule, exportDirectory->AddressOfFunctions);

	uintptr_t search = 0;
	for (int i = 0; i < exportDirectory->NumberOfNames; i++)
	{
		char* name = MakePtr(char*, hModule, pNames[i]);
		if (stricmp(lpProcName, name) == 0)
		{
			WORD ordinal = pOrdinals[i];
			search = MakePtr(uintptr_t, hModule, pFunctions[ordinal]);
		}
	}

	//Check if forwarded
	uintptr_t base = (uintptr_t)hModule + directory->VirtualAddress;
	if ((search >= base) && (search < (base + directory->Size)))
	{
		Assert(false);
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

	return search;
}

void Loader::KernelExports(void* address, void* kernelAddress)
{
	uintptr_t baseAddress = (uintptr_t)address;

	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)baseAddress;
	IMAGE_NT_HEADERS64* ntHeader = (PIMAGE_NT_HEADERS64)(baseAddress + dosHeader->e_lfanew);
	
	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	Print("import size: 0x%016x\n", importDirectory.Size);
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, baseAddress, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePtr(char*, baseAddress, importDescriptor->Name);
			Print("    %s\n", module);
			if (stricmp(module, "mosrt.dll") == 0)
			{
				Handle hModule = kernelAddress;

				PIMAGE_THUNK_DATA pThunkData = MakePtr(PIMAGE_THUNK_DATA, baseAddress, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePtr(PIMAGE_IMPORT_BY_NAME, baseAddress, pThunkData->u1.AddressOfData);

					pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
					Print("Patched %s to 0x%016x\n", (char*)pImportByName->Name, pThunkData->u1.Function);
					pThunkData++;

				}
			}
			importDescriptor++;
		}
	}
}
