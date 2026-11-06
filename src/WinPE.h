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
	
	//Validates the DOS and NT signatures and returns the NT headers
	inline const IMAGE_NT_HEADERS64* GetNtHeaders(const void* const imageBase)
	{
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		return ntHeader;
	}

	//Whether length bytes at rva lie in an image of imageSize bytes
	inline bool IsInImage(const size_t imageSize, const size_t rva, const size_t length)
	{
		return rva <= imageSize && length <= imageSize - rva;
	}

	//For an image its process can overwrite: the NT headers, or null unless they, the DOS header and the section table lie in its imageSize bytes and carry their signatures
	inline const IMAGE_NT_HEADERS64* GetNtHeaders(const void* const imageBase, const size_t imageSize)
	{
		const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
		if (!IsInImage(imageSize, 0, sizeof(IMAGE_DOS_HEADER)) || dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return nullptr;

		const LONG ntOffset = dosHeader->e_lfanew;
		if (ntOffset < 0 || !IsInImage(imageSize, (size_t)ntOffset, sizeof(IMAGE_NT_HEADERS64)))
			return nullptr;

		const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, ntOffset);
		const size_t sectionsOffset = (uintptr_t)IMAGE_FIRST_SECTION(ntHeader) - (uintptr_t)imageBase;
		if (ntHeader->Signature != IMAGE_NT_SIGNATURE || !IsInImage(imageSize, sectionsOffset, (size_t)ntHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER)))
			return nullptr;

		return ntHeader;
	}

	//For an image in mappedSize bytes of process memory: its SizeOfImage, or 0 unless its x64 headers and the whole image lie in those bytes
	inline size_t GetMappedImageSize(const void* const imageBase, const size_t mappedSize)
	{
		const IMAGE_NT_HEADERS64* ntHeader = GetNtHeaders(imageBase, mappedSize);
		if (!ntHeader || ntHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 || ntHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			return 0;

		const size_t imageSize = ntHeader->OptionalHeader.SizeOfImage;
		return (imageSize <= mappedSize) ? imageSize : 0;
	}

	inline DWORD GetEntryPoint(const void* const imageBase)
	{
		return GetNtHeaders(imageBase)->OptionalHeader.AddressOfEntryPoint;
	}
	inline DWORD GetSizeOfImage(const void* const imageBase)
	{
		return GetNtHeaders(imageBase)->OptionalHeader.SizeOfImage;
	}

	inline bool IsConsole(const void* const imageBase)
	{
		return GetNtHeaders(imageBase)->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
	}

	inline ULONGLONG GetStackSize(const void* const imageBase)
	{
		return GetNtHeaders(imageBase)->OptionalHeader.SizeOfStackReserve;
	}

	inline const IMAGE_SECTION_HEADER* GetPESection(const void* const imageBase, const char* const name)
	{
		const IMAGE_NT_HEADERS64* ntHeader = GetNtHeaders(imageBase);

		//Find section
		const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);
		for (WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
		{
			if (CStr((const char*)section[i].Name, IMAGE_SIZEOF_SHORT_NAME) == CStr(name))
				return &section[i];
		}

		return nullptr;
	}

	inline const IMAGE_SECTION_HEADER* GetPESection(const void* const imageBase, const uint32_t index)
	{
		const IMAGE_NT_HEADERS64* ntHeader = GetNtHeaders(imageBase);
		Assert(index < ntHeader->FileHeader.NumberOfSections);

		//Find section
		const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);
		return &section[index];
	}

	typedef void (*CrtInitializer)();

	//Calls each static initializer in the .CRT section's null-terminated list, returns false if there is no .CRT section
	inline bool RunCrtInitializers(const void* const imageBase)
	{
		const IMAGE_SECTION_HEADER* crtSection = GetPESection(imageBase, ".CRT");
		if (crtSection == nullptr)
			return false;

		CrtInitializer* initializer = MakePointer<CrtInitializer*>(imageBase, crtSection->VirtualAddress);
		while (*initializer)
		{
			(*initializer)();
			initializer++;
		}

		return true;
	}

	inline void* GetProcAddress(const void* const imageBase, const char* const procName)
	{
		const IMAGE_NT_HEADERS64* ntHeader = GetNtHeaders(imageBase);
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
			if (strcmp(name, procName) == 0)
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

		return (void*)search;
	}

	//ReactOS::reactos\dll\win32\dbghelp\msc.c
	#define MAKESIG(a,b,c,d)        ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
	#define CODEVIEW_RSDS_SIG       MAKESIG('R','S','D','S')	

	//The RSDS CodeView record naming the image's PDB and its GUID and age; null for an image linked without a PDB (no debug directory, or no RSDS entry in it), or when its headers (section table included), debug directory or record and name don't lie in its imageSize bytes
	inline const OMFSignatureRSDS* GetCodeView(const void* const imageBase, const size_t imageSize)
	{
		const IMAGE_NT_HEADERS64* ntHeader = GetNtHeaders(imageBase, imageSize);
		if (!ntHeader)
			return nullptr;

		const IMAGE_DATA_DIRECTORY directory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
		if ((directory.Size == 0) || (directory.VirtualAddress == 0) || !IsInImage(imageSize, directory.VirtualAddress, directory.Size))
			return nullptr;

		//The linker also emits feature, POGO and repro entries; CodeView isn't guaranteed to be first
		const IMAGE_DEBUG_DIRECTORY* debugDirectory = MakePointer<const IMAGE_DEBUG_DIRECTORY*>(imageBase, directory.VirtualAddress);
		const size_t count = directory.Size / sizeof(IMAGE_DEBUG_DIRECTORY);
		for (size_t i = 0; i < count; i++)
		{
			const IMAGE_DEBUG_DIRECTORY& entry = debugDirectory[i];
			const DWORD rva = entry.AddressOfRawData;
			if (entry.Type != IMAGE_DEBUG_TYPE_CODEVIEW || rva == 0 || !IsInImage(imageSize, rva, sizeof(OMFSignatureRSDS)))
				continue;

			const OMFSignatureRSDS* rsds = MakePointer<const OMFSignatureRSDS*>(imageBase, rva);
			if (*(DWORD*)rsds != CODEVIEW_RSDS_SIG)
				continue;

			//Its name runs to a NUL before the image ends
			const size_t nameOffset = rva + (size_t)(rsds->name - (const char*)rsds);
			size_t length = 0;
			while (nameOffset + length < imageSize && rsds->name[length] != '\0')
				length++;
			if (nameOffset + length == imageSize)
				continue;

			return rsds;
		}

		return nullptr;
	}

	//Null for an image linked without a PDB, or one whose CodeView record GetCodeView can't trust
	inline const char* GetPdbName(const void* const imageBase, const size_t imageSize)
	{
		const OMFSignatureRSDS* const rsds = GetCodeView(imageBase, imageSize);
		return rsds ? rsds->name : nullptr;
	}
}

