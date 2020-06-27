#include <MetalOS.h>
#include <WindowsPE.h>
#include <crt_string.h>

#define ReturnNullIfNot(x) if (!(x)) return nullptr;

extern "C" Handle LoadLibrary(const char* lpLibFileName)
{
	Handle file = CreateFile(lpLibFileName, GenericAccess::Read);
	ReturnNullIfNot(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	ReturnNullIfNot(ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	ReturnNullIfNot(read == sizeof(IMAGE_DOS_HEADER));
	ReturnNullIfNot(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	ReturnNullIfNot(SetFilePosition(file, dosHeader.e_lfanew));
	ReturnNullIfNot(ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	ReturnNullIfNot(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		ReturnNullIfNot(false);
	}

	//Reserve space for image
	void* moduleBase = VirtualAlloc((void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWriteExecute);
	if (moduleBase == nullptr)//we cant, so get any address
		moduleBase = VirtualAlloc(nullptr, peHeader.OptionalHeader.SizeOfImage, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWriteExecute);
	ReturnNullIfNot(moduleBase == nullptr);

	//Read in headers
	ReturnNullIfNot(SetFilePosition(file, 0));
	ReturnNullIfNot(ReadFile(file, moduleBase, peHeader.OptionalHeader.SizeOfHeaders, &read));
	ReturnNullIfNot(read == peHeader.OptionalHeader.SizeOfHeaders);

	//Update pointer to loaded module
	PIMAGE_NT_HEADERS64 pNtHeader = MakePtr(PIMAGE_NT_HEADERS64, moduleBase, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)moduleBase + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			ReturnNullIfNot(SetFilePosition(file, section[i].PointerToRawData));
			ReturnNullIfNot(ReadFile(file, (void*)destination, rawSize, &read));
			ReturnNullIfNot(read == section[i].SizeOfRawData);
		}
	}

	//Imports
	IMAGE_DATA_DIRECTORY importDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, moduleBase, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePtr(char*, moduleBase, importDescriptor->Name);
			Handle hModule = LoadLibrary(module);

			PIMAGE_THUNK_DATA pThunkData = MakePtr(PIMAGE_THUNK_DATA, moduleBase, importDescriptor->FirstThunk);
			while (pThunkData->u1.AddressOfData)
			{
				PIMAGE_IMPORT_BY_NAME pImportByName = MakePtr(PIMAGE_IMPORT_BY_NAME, moduleBase, pThunkData->u1.AddressOfData);

				pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
				pThunkData++;
			}
			importDescriptor++;
		}
	}

	//Relocations
	IMAGE_DATA_DIRECTORY relocationDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if (((uintptr_t)moduleBase != pNtHeader->OptionalHeader.ImageBase) && (relocationDirectory.Size))
	{
		PIMAGE_BASE_RELOCATION pBaseRelocation = MakePtr(PIMAGE_BASE_RELOCATION, moduleBase, relocationDirectory.VirtualAddress);

		//Calculate relative shift
		uintptr_t delta = (uintptr_t)moduleBase - (uintptr_t)pNtHeader->OptionalHeader.ImageBase;

		DWORD bytes = 0;
		while (bytes < relocationDirectory.Size)
		{
			PDWORD locationBase = MakePtr(PDWORD, moduleBase, pBaseRelocation->VirtualAddress);
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

	//TODO: create thread to execute entrypoint

	return moduleBase;
}

extern "C" uintptr_t GetProcAddress(Handle hModule, const char* lpProcName)
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

	char** pNames = MakePtr(char**, hModule, exportDirectory->AddressOfNames);
	PWORD pOrdinals = MakePtr(PWORD, hModule, exportDirectory->AddressOfNameOrdinals);
	uintptr_t* pFunctions = MakePtr(uintptr_t*, hModule, exportDirectory->AddressOfFunctions);

	uintptr_t search = 0;
	for (int i = 0; i < exportDirectory->NumberOfNames; i++)
	{
		char* name = MakePtr(char*, hModule, pNames[i]);
		if (stricmp(lpProcName, name) == 0)
		{
			WORD ordinal = pOrdinals[i];
			search = MakePtr(DWORD, hModule, pFunctions[ordinal]);
		}
	}

	//Check if forwarded
	uintptr_t base = (uintptr_t)hModule + directory->VirtualAddress;
	if ((search >= base) && (search < (base + directory->Size)))
		return NULL;



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
