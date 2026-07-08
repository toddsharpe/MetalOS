#pragma once

#include "core_crt/string.h"
#include "Lib/System.h"
#include "user/MetalOS-RT/Loader.h"
#include "user/MetalOS-RT/Runtime.h"
#include "user/MetalOS.h"
#include "WinPE.h"

#define RetNullIfNot(x) if (!(x)) return nullptr;
#define RetNullIfFailed(x) if ((x) != SyscallResult::Success) return nullptr;

namespace Loader
{
	static const char DllMainName[] = "DllMain";
	
	typedef void (*CrtInitializer)();
	void CrtInit(const void* const imageBase)
	{
		const IMAGE_SECTION_HEADER* crtSection = WinPE::GetPESection(imageBase, ".CRT");
		if (crtSection != nullptr)
		{
			CrtInitializer* initializer = (CrtInitializer*)((uintptr_t)imageBase + crtSection->VirtualAddress);
			while (*initializer)
			{
				(*initializer)();
				initializer++;
			}
		}
	}
}

HModule LoadLibrary(char* lpLibFileName)
{
	CDebugPrintf(Runtime::IsDebug(), "LoadLibrary: %s\n", lpLibFileName);
	
	//Check if module is already loaded
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();
	Module* module = Runtime::GetLoadedModule(lpLibFileName);
	if (module != nullptr)
	{
		CDebugPrintf(Runtime::IsDebug(), "  Loaded at: 0x%016x\n", module->ImageBase);
		return (HModule)module->ImageBase;
	}
	
	//Load it
	HFile file = CreateFile(lpLibFileName, FileAccess::Read);
	RetNullIfNot(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	RetNullIfFailed(ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	RetNullIfNot(read == sizeof(IMAGE_DOS_HEADER));
	RetNullIfNot(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	RetNullIfFailed(SetFilePointer(file, dosHeader.e_lfanew, Seek::Begin, nullptr));
	RetNullIfFailed(ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	RetNullIfNot(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		RetNullIfNot(false);
	}

	//Reserve space for image
	void* moduleBase = VirtualAlloc((void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage);
	//TODO(tsharpe): Implement relocations (below). Until then assert address is the one in PE header.
	RetNullIfNot(moduleBase)
	CDebugPrintf(Runtime::IsDebug(), "  Loading at 0x%016x\n", moduleBase);

	//Read in headers
	RetNullIfFailed(SetFilePointer(file, 0, Seek::Begin, nullptr));
	RetNullIfFailed(ReadFile(file, moduleBase, peHeader.OptionalHeader.SizeOfHeaders, &read));
	RetNullIfNot(read == peHeader.OptionalHeader.SizeOfHeaders);

	//Update pointer to loaded module
	PIMAGE_NT_HEADERS64 pNtHeader = MakePointer<PIMAGE_NT_HEADERS64>(moduleBase, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		void* destination = MakePointer<void*>(moduleBase, section[i].VirtualAddress);

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			RetNullIfFailed(SetFilePointer(file, section[i].PointerToRawData, Seek::Begin, nullptr));
			RetNullIfFailed(ReadFile(file, destination, rawSize, &read));
			RetNullIfNot(read == section[i].SizeOfRawData);
		}
	}

	CloseHandle(file);

	//Imports
	IMAGE_DATA_DIRECTORY importDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>(moduleBase, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePointer<char*>(moduleBase, importDescriptor->Name);
			HModule hModule = LoadLibrary(module);
			Assert(hModule);

			PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>(moduleBase, importDescriptor->FirstThunk);
			while (pThunkData->u1.AddressOfData)
			{
				PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(moduleBase, pThunkData->u1.AddressOfData);

				pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
				CDebugPrintf(Runtime::IsDebug(), "  Patched %s::%s to 0x%016x\n", module, (char*)pImportByName->Name, pThunkData->u1.Function);
				pThunkData++;
			}
			importDescriptor++;
		}
	}

	//Relocations
	//IMAGE_DATA_DIRECTORY relocationDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	//if (((uintptr_t)moduleBase != pNtHeader->OptionalHeader.ImageBase) && (relocationDirectory.Size))
	//{
	//	PIMAGE_BASE_RELOCATION pBaseRelocation = MakePtr(PIMAGE_BASE_RELOCATION, moduleBase, relocationDirectory.VirtualAddress);

	//	//Calculate relative shift
	//	uintptr_t delta = (uintptr_t)moduleBase - (uintptr_t)pNtHeader->OptionalHeader.ImageBase;

	//	DWORD bytes = 0;
	//	while (bytes < relocationDirectory.Size)
	//	{
	//		PDWORD locationBase = MakePtr(PDWORD, moduleBase, pBaseRelocation->VirtualAddress);
	//		PWORD locationData = MakePtr(PWORD, pBaseRelocation, sizeof(IMAGE_BASE_RELOCATION));

	//		DWORD count = (pBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
	//		for (DWORD i = 0; i < count; i++)
	//		{
	//			int type = (*locationData >> 12);
	//			int value = (*locationData & 0x0FFF);

	//			switch (type)
	//			{
	//			case IMAGE_REL_BASED_ABSOLUTE:
	//				break;

	//			case IMAGE_REL_BASED_HIGHLOW:
	//				*MakePtr(PDWORD, locationBase, value) += delta;
	//				break;
	//			}

	//			locationData++;
	//		}

	//		bytes += pBaseRelocation->SizeOfBlock;
	//		pBaseRelocation = (PIMAGE_BASE_RELOCATION)locationData;
	//	}
	//}

	//Run static initializers
	Loader::CrtInit(moduleBase);

	//Mark module as in the process
	peb->LoadedModules[peb->ModuleIndex].ImageBase = moduleBase;
	strcpy(peb->LoadedModules[peb->ModuleIndex].Name, lpLibFileName);
	peb->ModuleIndex++;

	//Execute entry point
	//TODO: thread
	DllMainCall main = (DllMainCall)GetProcAddress((HModule)moduleBase, Loader::DllMainName);
	main((HModule)moduleBase);

	return (HModule)moduleBase;
}

uintptr_t GetProcAddress(HModule hModule, const char* lpProcName)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = MakePointer<PIMAGE_DOS_HEADER>(hModule, 0);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(hModule, dosHeader->e_lfanew);
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	if (ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		return NULL;

	PIMAGE_DATA_DIRECTORY directory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if ((directory->Size == 0) || (directory->VirtualAddress == 0))
		return NULL;

	PIMAGE_EXPORT_DIRECTORY exportDirectory = MakePointer<PIMAGE_EXPORT_DIRECTORY>(hModule, directory->VirtualAddress);

	PDWORD pNames = MakePointer<PDWORD>(hModule, exportDirectory->AddressOfNames);
	PWORD pOrdinals = MakePointer<PWORD>(hModule, exportDirectory->AddressOfNameOrdinals);
	PDWORD pFunctions = MakePointer<PDWORD>(hModule, exportDirectory->AddressOfFunctions);

	uintptr_t search = 0;
	for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++)
	{
		char* name = MakePointer<char*>(hModule, pNames[i]);
		if (_stricmp(lpProcName, name) == 0)
		{
			WORD ordinal = pOrdinals[i];
			search = MakePointer<uintptr_t>(hModule, pFunctions[ordinal]);
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
