#include <cstdint>
#include <WindowsPE.h>
#include <MetalOS.h>
#include "Runtime.h"
#include <crt_string.h>

extern "C" int main(int argc, char** argv);
typedef int (*ProcessEntry)();

//Just to build
extern "C" int WinMainCRTStartup() //should be placed at entry point by msvc
{
	return main(0, nullptr);
}

//Using load library here doesn't work, since thats an import and this is before imports are loaded. Maybe OS needs to load this library?

extern "C" __declspec(dllexport) void InitProcess()
{
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();
	uintptr_t baseAddress = peb->BaseAddress;

	//Perform library loading
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)baseAddress;
	IMAGE_NT_HEADERS64* ntHeader = (PIMAGE_NT_HEADERS64)(baseAddress + dosHeader->e_lfanew);

	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, baseAddress, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePtr(char*, baseAddress, importDescriptor->Name);
			Handle hModule = LoadLibrary(module);
			if (stricmp(module, "mosapi.dll") != 0)
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

