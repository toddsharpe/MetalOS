#pragma once

#include "core_crt/string.h"
#include "Lib/System.h"
#include "user/MetalOS-RT/Loader.h"
#include "user/MetalOS-RT/Runtime.h"
#include "user/MetalOS.h"
#include "WinPE.h"
#include "Assert.h"

#define RetNullIfNot(x) if (!(x)) return nullptr;
#define RetNullIfFailed(x) if ((x) != SyscallResult::Success) return nullptr;

namespace Loader
{
	static const char DllMainName[] = "DllMain";
	
	void ResolveImports(void* const imageBase, const char* const skipModule)
	{
		IMAGE_DATA_DIRECTORY importDirectory = WinPE::GetNtHeaders(imageBase)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (importDirectory.Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>(imageBase, importDirectory.VirtualAddress);

			while (importDescriptor->Name)
			{
				char* module = MakePointer<char*>(imageBase, importDescriptor->Name);
				Assert(module);
				HModule hModule = LoadLibrary(module);
				Assert(hModule);

				if ((skipModule == nullptr) || (_stricmp(module, skipModule) != 0))
				{
					PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>(imageBase, importDescriptor->FirstThunk);
					while (pThunkData->u1.AddressOfData)
					{
						PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(imageBase, pThunkData->u1.AddressOfData);

						pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
						CDebugPrintf(Runtime::IsDebug(), "  Patched %s::%s to 0x%016x\n", module, (char*)pImportByName->Name, pThunkData->u1.Function);
						pThunkData++;
					}
				}

				importDescriptor++;
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
	//TODO(tsharpe): Implement relocations. Until then assert address is the one in PE header.
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

	//Before its imports, initializers and DllMain run, so a fault in any of them traces with its symbols (failing only costs the trace its symbols)
	RegisterModule(lpLibFileName, moduleBase);

	//Imports
	Loader::ResolveImports(moduleBase, nullptr);

	//Run static initializers
	WinPE::RunCrtInitializers(moduleBase);

	//Mark module as in the process
	peb->LoadedModules[peb->ModuleIndex].ImageBase = moduleBase;
	strncpy(peb->LoadedModules[peb->ModuleIndex].Name, lpLibFileName, MaxModuleName - 1);
	peb->LoadedModules[peb->ModuleIndex].Name[MaxModuleName - 1] = '\0';
	peb->ModuleIndex++;

	//Execute entry point
	//TODO: thread
	DllMainCall main = (DllMainCall)GetProcAddress((HModule)moduleBase, Loader::DllMainName);
	const bool result = main((HModule)moduleBase, DllReason::ProcessAttach);
	if (!result)
	{
		FreeLibrary((HModule)moduleBase);
		return nullptr;
	}

	return (HModule)moduleBase;
}

bool FreeLibrary(HModule hModule)
{
	CDebugPrintf(Runtime::IsDebug(), "FreeLibrary: 0x%016x\n", hModule);

	//Execute entry point - do not check return value
	DllMainCall main = (DllMainCall)GetProcAddress(hModule, Loader::DllMainName);
	main(hModule, DllReason::ProcessDetach);

	//Free region, which also drops the kernel's record of the module
	const bool result = VirtualFree(hModule);
	Assert(result);

	return true;
}

uintptr_t GetProcAddress(HModule hModule, const char* lpProcName)
{
	//A module without exports resolves to null here, WinPE would assert
	if (WinPE::GetNtHeaders(hModule)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		return NULL;

	return (uintptr_t)WinPE::GetProcAddress(hModule, CString(lpProcName));
}
