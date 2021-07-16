#include "PortableExecutable.h"
#include "Main.h"

size_t PortableExecutable::GetSizeOfImage(const Handle hModule)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = MakePtr(PIMAGE_DOS_HEADER, hModule, 0);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	PIMAGE_NT_HEADERS64 ntHeader = MakePtr(PIMAGE_NT_HEADERS64, hModule, dosHeader->e_lfanew);
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	return ntHeader->OptionalHeader.SizeOfImage;
}

uintptr_t PortableExecutable::GetEntryPoint(const Handle hModule)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = MakePtr(PIMAGE_DOS_HEADER, hModule, 0);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	PIMAGE_NT_HEADERS64 ntHeader = MakePtr(PIMAGE_NT_HEADERS64, hModule, dosHeader->e_lfanew);
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	return ntHeader->OptionalHeader.AddressOfEntryPoint;
}

PIMAGE_SECTION_HEADER PortableExecutable::GetPESection(const std::string& name, const uintptr_t ImageBase)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)((uint64_t)ImageBase + dosHeader->e_lfanew);

	//Find section
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		if ((char*)&section[i].Name == name)
			return &section[i];
	}

	Assert(false);
	return nullptr;
}

PIMAGE_SECTION_HEADER PortableExecutable::GetPESection(const uint32_t index, const uintptr_t ImageBase)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)((uint64_t)ImageBase + dosHeader->e_lfanew);

	Assert(index < pNtHeader->FileHeader.NumberOfSections);

	//Find section
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	return &section[index];
}

uintptr_t PortableExecutable::GetProcAddress(Handle hModule, const char* lpProcName)
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

//ReactOS::reactos\dll\win32\dbghelp\msc.c
#define MAKESIG(a,b,c,d)        ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define CODEVIEW_RSDS_SIG       MAKESIG('R','S','D','S')
const char* PortableExecutable::GetPdbName(Handle hModule)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = MakePtr(PIMAGE_DOS_HEADER, hModule, 0);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	PIMAGE_NT_HEADERS64 ntHeader = MakePtr(PIMAGE_NT_HEADERS64, hModule, dosHeader->e_lfanew);
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
		return nullptr;

	if (ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		return nullptr;

	PIMAGE_DATA_DIRECTORY directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	if ((directory->Size == 0) || (directory->VirtualAddress == 0))
		return nullptr;

	PIMAGE_DEBUG_DIRECTORY debugDirectory = MakePtr(PIMAGE_DEBUG_DIRECTORY, hModule, directory->VirtualAddress);
	Assert(debugDirectory->Type == IMAGE_DEBUG_TYPE_CODEVIEW);
	
	void* address = MakePtr(void*, hModule, debugDirectory->AddressOfRawData);
	Assert(*(DWORD*)address == CODEVIEW_RSDS_SIG);

	const OMFSignatureRSDS* rsds = (const OMFSignatureRSDS*)address;
	return rsds->name;
}
