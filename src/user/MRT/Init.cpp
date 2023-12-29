#include "MetalOS.h"
#include "string.h"
#include <windows/types.h>
#include <windows/winnt.h>
#include "Runtime.h"
#include "Assert.h"
#include "user/MetalOS.Types.h"
#include "Loader.h"

typedef int (*ProcessEntry)(int argc, char** argv);

extern "C" __declspec(dllexport) void InitProcess()//Rename: init module?
{
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();
	uintptr_t baseAddress = peb->BaseAddress;

	DebugPrintf("InitProcess\n");

	//Perform library loading
	IMAGE_DOS_HEADER* dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
	IMAGE_NT_HEADERS64* ntHeader = (PIMAGE_NT_HEADERS64)(baseAddress + dosHeader->e_lfanew);

	//Imports
	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>((void*)baseAddress, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePointer<char*>((void*)baseAddress, importDescriptor->Name);
			Handle hModule = Loader::LoadLibrary(module);
			Assert(hModule);

			//Kernel does RT imports. Due to the PE unions, imports can't be done more than once.
			if (_stricmp(module, "mosrt.dll") != 0)
			{
				PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>((void*)baseAddress, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>((void*)baseAddress, pThunkData->u1.AddressOfData);

					pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
					DebugPrintf("  Patched %s to 0x%016x\n", (char*)pImportByName->Name, pThunkData->u1.Function);
					pThunkData++;
				}
			}

			importDescriptor++;
		}
	}

	//TODO: TLS

	Loader::CrtInit((Handle)baseAddress);

	//Call entry
	ProcessEntry entry = MakePointer<ProcessEntry>((void*)baseAddress, ntHeader->OptionalHeader.AddressOfEntryPoint);
	int ret = entry(0, nullptr);

	//Kill process
	ExitProcess(0);
}

extern "C" __declspec(dllexport) void InitThread()
{
	DebugPrint("InitThread\n");

	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	teb->ThreadStart(teb->Arg);

	//Kill thread
	ExitThread(0);
}
