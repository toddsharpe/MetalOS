#pragma once

#include "windows/types.h"
#include "windows/winnt.h"
#include "Lib/System.h"
#include "Lib/String.h"

//https://deplinenoise.wordpress.com/2013/06/14/getting-your-pdb-name-from-a-running-executable-windows/
//https://gist.github.com/gimelfarb/8642282
namespace WinPE
{
	//GuidDef.h
	typedef struct {
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	} GUID;

	//ReactOS::sdk\include\reactos\wine\mscvpdb.h
	typedef struct OMFSignatureRSDS
	{
		char        Signature[4];
		GUID        guid;
		DWORD       age;
		CHAR        name[1];
	} OMFSignatureRSDS;
	
	DWORD GetEntryPoint(const void* const imageBase)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		return ntHeader->OptionalHeader.AddressOfEntryPoint;
	}
	DWORD GetSizeOfImage(const void* const imageBase)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		return ntHeader->OptionalHeader.SizeOfImage;
	}

	bool IsConsole(const void* const imageBase)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		return ntHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
	}

	ULONGLONG GetStackSize(const void* const imageBase)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		return ntHeader->OptionalHeader.SizeOfStackReserve;
	}

	const IMAGE_SECTION_HEADER* GetPESection(const void* const imageBase, const CString& name)
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
			if (name == CString((char*)&section[i].Name))
				return &section[i];
		}

		return nullptr;
	}

	const IMAGE_SECTION_HEADER* GetPESection(const void* const imageBase, const uint32_t index)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		Assert(index < ntHeader->FileHeader.NumberOfSections);

		//Find section
		const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);
		return &section[index];
	}

	void* GetProcAddress(const void* const imageBase, const CString& procName)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		Assert(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size);

		const IMAGE_DATA_DIRECTORY* directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		if ((directory->Size == 0) || (directory->VirtualAddress == 0))
			return NULL;

		const IMAGE_EXPORT_DIRECTORY* exportDirectory = MakePointer<const IMAGE_EXPORT_DIRECTORY*>(imageBase, directory->VirtualAddress);

		const DWORD* pNames = MakePointer<const DWORD*>(imageBase, exportDirectory->AddressOfNames);
		const WORD* pOrdinals = MakePointer<const WORD*>(imageBase, exportDirectory->AddressOfNameOrdinals);
		const DWORD* pFunctions = MakePointer<const DWORD*>(imageBase, exportDirectory->AddressOfFunctions);

		uintptr_t search = 0;
		for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++)
		{
			char* name = MakePointer<char*>(imageBase, pNames[i]);
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

	//ReactOS::reactos\dll\win32\dbghelp\msc.c
	#define MAKESIG(a,b,c,d)        ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
	#define CODEVIEW_RSDS_SIG       MAKESIG('R','S','D','S')	

	const char* GetPdbName(const void* const imageBase)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		Assert(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);

		const IMAGE_DATA_DIRECTORY* directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
		if ((directory->Size == 0) || (directory->VirtualAddress == 0))
			return nullptr;

		const IMAGE_DEBUG_DIRECTORY* debugDirectory = MakePointer<const IMAGE_DEBUG_DIRECTORY*>(imageBase, directory->VirtualAddress);
		AssertEqual(debugDirectory->Type, IMAGE_DEBUG_TYPE_CODEVIEW);
		
		const void* address = MakePointer<const void*>(imageBase, debugDirectory->AddressOfRawData);
		AssertEqual(*(DWORD*)address, CODEVIEW_RSDS_SIG);

		const OMFSignatureRSDS* rsds = (const OMFSignatureRSDS*)address;
		return rsds->name;
	}
	bool Contains(const void* const imageBase, const uintptr_t ip)
	{
		//Headers
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		const uintptr_t imageEnd = (uintptr_t)imageBase + ntHeader->OptionalHeader.SizeOfImage;
		return ((ip >= (uintptr_t)imageBase) && (ip < imageEnd));
	}
}

