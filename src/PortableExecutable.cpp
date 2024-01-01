#include "PortableExecutable.h"
#include "Assert.h"
#include "user/MetalOS.System.h"

void* PortableExecutable::GetEntryPoint(const void* const imageBase)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);
	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	void* const entry = MakePointer<void*>(imageBase, ntHeader->OptionalHeader.AddressOfEntryPoint);
	return entry;
}

uint32_t PortableExecutable::GetSizeOfImage(const void* const imageBase)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);
	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	return ntHeader->OptionalHeader.SizeOfImage;
}

void* PortableExecutable::GetProcAddress(const void* const imageBase, const char* const procName)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);
	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	Assert(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size);

	const IMAGE_DATA_DIRECTORY* directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	Assert(directory->Size);
	Assert(directory->VirtualAddress);

	const IMAGE_EXPORT_DIRECTORY* exportDirectory = MakePointer<const IMAGE_EXPORT_DIRECTORY*>(imageBase, directory->VirtualAddress);

	PDWORD pNames = MakePointer<PDWORD>(imageBase, exportDirectory->AddressOfNames);
	PWORD pOrdinals = MakePointer<PWORD>(imageBase, exportDirectory->AddressOfNameOrdinals);
	PDWORD pFunctions = MakePointer<PDWORD>(imageBase, exportDirectory->AddressOfFunctions);

	uintptr_t search = 0;
	for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++)
	{
		const char* name = MakePointer<const char*>(imageBase, pNames[i]);
		if (strcmp(procName,name) == 0)
		{
			const WORD ordinal = pOrdinals[i];
			search = MakePointer<uintptr_t>(imageBase, pFunctions[ordinal]);
			break;
		}
	}

	//Check if forwarded
	uintptr_t base = (uintptr_t)imageBase + directory->VirtualAddress;
	const bool forwarded = ((search >= base) && (search < (base + directory->Size)));
	Assert(!forwarded);

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

	Assert(search);
	return (void*)search;
}

//ReactOS::reactos\dll\win32\dbghelp\msc.c
#define MAKESIG(a,b,c,d)        ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define CODEVIEW_RSDS_SIG       MAKESIG('R','S','D','S')
const char* PortableExecutable::GetPdbName(const void* const imageBase)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);
	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	Assert(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);

	const IMAGE_DATA_DIRECTORY* directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	Assert(directory->Size);
	Assert(directory->VirtualAddress);

	const IMAGE_DEBUG_DIRECTORY* debugDirectory = MakePointer<const IMAGE_DEBUG_DIRECTORY*>(imageBase, directory->VirtualAddress);
	AssertEqual(debugDirectory->Type, IMAGE_DEBUG_TYPE_CODEVIEW);

	const void* address = MakePointer<void*>(imageBase, debugDirectory->AddressOfRawData);
	AssertEqual(*(DWORD*)address, CODEVIEW_RSDS_SIG);

	const OMFSignatureRSDS* rsds = (const OMFSignatureRSDS*)address;
	return rsds->name;
}

const IMAGE_SECTION_HEADER* PortableExecutable::GetPESection(const void* const imageBase, const char* name)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);
	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	//Find section
	const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);
	for (WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		if (strcmp((char*)&section[i].Name, name) == 0)
			return &section[i];
	}

	Assert(false);
	return nullptr;
}

const IMAGE_SECTION_HEADER* PortableExecutable::GetPESection(const void* const imageBase, const uint32_t index)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);
	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	Assert(index < ntHeader->FileHeader.NumberOfSections);

	//Find section
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeader);
	return &section[index];
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
