#include "Kernel/Kernel.h"
#include "Assert.h"
#include <windows/types.h>
#include <windows/winnt.h>

#include "PortableExecutable.h"

DWORD PortableExecutable::GetSizeOfImage(void* const imageBase)
{
	//Headers
	const PIMAGE_DOS_HEADER dosHeader = static_cast<const PIMAGE_DOS_HEADER>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	return ntHeader->OptionalHeader.SizeOfImage;
}

DWORD PortableExecutable::GetEntryPoint(void* const imageBase)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	return ntHeader->OptionalHeader.AddressOfEntryPoint;
}

PIMAGE_SECTION_HEADER PortableExecutable::GetPESection(void* const imageBase, const std::string& name)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	//Find section
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeader);
	for (WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		if ((char*)&section[i].Name == name)
			return &section[i];
	}

	Assert(false);
	return nullptr;
}

PIMAGE_SECTION_HEADER PortableExecutable::GetPESection(void* const imageBase, const uint32_t index)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	Assert(index < ntHeader->FileHeader.NumberOfSections);

	//Find section
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeader);
	return &section[index];
}

void* PortableExecutable::GetProcAddress(void* const imageBase, const std::string& procName)
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

	PIMAGE_EXPORT_DIRECTORY exportDirectory = MakePointer<PIMAGE_EXPORT_DIRECTORY> (imageBase, directory->VirtualAddress);

	PDWORD pNames = MakePointer<PDWORD>(imageBase, exportDirectory->AddressOfNames);
	PWORD pOrdinals = MakePointer<PWORD>(imageBase, exportDirectory->AddressOfNameOrdinals);
	PDWORD pFunctions = MakePointer<PDWORD>(imageBase, exportDirectory->AddressOfFunctions);

	uintptr_t search = 0;
	for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++)
	{
		char* name = MakePointer<char*>(imageBase, pNames[i]);
		if (procName == name)
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

//ReactOS::reactos\dll\win32\dbghelp\msc.c
#define MAKESIG(a,b,c,d)        ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define CODEVIEW_RSDS_SIG       MAKESIG('R','S','D','S')
const char* PortableExecutable::GetPdbName(void* const imageBase)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	Assert(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);

	PIMAGE_DATA_DIRECTORY directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	if ((directory->Size == 0) || (directory->VirtualAddress == 0))
		return nullptr;

	PIMAGE_DEBUG_DIRECTORY debugDirectory = MakePointer<PIMAGE_DEBUG_DIRECTORY>(imageBase, directory->VirtualAddress);
	AssertEqual(debugDirectory->Type, IMAGE_DEBUG_TYPE_CODEVIEW);
	
	const void* address = MakePointer<void*>(imageBase, debugDirectory->AddressOfRawData);
	AssertEqual(*(DWORD*)address, CODEVIEW_RSDS_SIG);

	const OMFSignatureRSDS* rsds = (const OMFSignatureRSDS*)address;
	return rsds->name;
}

bool PortableExecutable::Contains(const void* const imageBase, const uintptr_t ip)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	const uintptr_t imageEnd = (uintptr_t)imageBase + ntHeader->OptionalHeader.SizeOfImage;
	return ((ip >= (uintptr_t)imageBase) && (ip < imageEnd));
}
