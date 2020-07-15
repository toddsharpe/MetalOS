#include <cstdint>
#include <WindowsPE.h>
#include <MetalOS.h>
#include "Runtime.h"
#include <string.h>
#include <Debug.h>

extern "C" int main(int argc, char** argv);
typedef int (*ProcessEntry)();
typedef void (*CrtInitializer)();

PIMAGE_SECTION_HEADER GetPESection(Handle imageBase, const char* name)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)imageBase;
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)((uint64_t)imageBase + dosHeader->e_lfanew);

	//Find section
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		if (strcmp((char*)&section[i].Name, name) == 0)
			return &section[i];
	}

	return nullptr;
}

void ExecuteStaticInitializers(Handle moduleBase)
{
	PIMAGE_SECTION_HEADER crtSection = GetPESection(moduleBase, ".CRT");
	if (crtSection != nullptr)
	{
		CrtInitializer* initializer = (CrtInitializer*)((uintptr_t)moduleBase + crtSection->VirtualAddress);
		while (*initializer)
		{
			(*initializer)();
			initializer++;
		}
	}
}

//Just to build
extern "C" int WinMainCRTStartup() //should be placed at entry point by msvc
{
	return main(0, nullptr);
}

extern "C" __declspec(dllexport) void InitProcess()//Rename: init module?
{
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();
	uintptr_t baseAddress = peb->BaseAddress;

	//Finish loading mosrt
	Handle mosrt = LoadLibrary("mosrt.dll");
	ExecuteStaticInitializers(mosrt);

	//Execute entry point
	//TODO: thread
	DllMainCall main = (DllMainCall)GetProcAddress(mosrt, DllMainName);
	main(mosrt, DllEntryReason::ProcessAttach);

	//Perform library loading
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)baseAddress;
	IMAGE_NT_HEADERS64* ntHeader = (PIMAGE_NT_HEADERS64)(baseAddress + dosHeader->e_lfanew);

	//Finish imports (mosrt.dll was loaded by kernel)
	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, baseAddress, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePtr(char*, baseAddress, importDescriptor->Name);
			Handle hModule = LoadLibrary(module);
			if (hModule == nullptr)
				ExitProcess(-1);

			if (stricmp(module, "mosrt.dll") != 0)
			{
				PIMAGE_THUNK_DATA pThunkData = MakePtr(PIMAGE_THUNK_DATA, baseAddress, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePtr(PIMAGE_IMPORT_BY_NAME, baseAddress, pThunkData->u1.AddressOfData);

					pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
					pThunkData++;
				}
			}

			importDescriptor++;
		}
	}

	//TODO: TLS

	//TODO: C++ statics
	ExecuteStaticInitializers((Handle)baseAddress);

	//Call entry
	ProcessEntry entry = MakePtr(ProcessEntry, baseAddress, ntHeader->OptionalHeader.AddressOfEntryPoint);
	int ret = entry();

	//Kill process
	ExitProcess(0);
}

extern "C" __declspec(dllexport) void InitThread()
{
	DebugPrint("InitThread");

	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	teb->ThreadStart(teb->Arg);

	//Kill thread
	ExitThread(0);
}

