#include "MetalOS.h"
#include "string.h"
#include <windows/types.h>
#include <windows/winnt.h>
#include "Runtime.h"
#include "Assert.h"
#include "user/MetalOS.Types.h"
#include "Loader.h"
#include <vector>
#include <string>

typedef int (*ProcessEntry)(int argc, char** argv);
extern "C" __declspec(dllexport) void InitProcess()//Rename: init module?
{
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();

	DebugPrintf("InitProcess\n");

	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const PIMAGE_DOS_HEADER>(peb->ImageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(peb->ImageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	//Imports
	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>(peb->ImageBase, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePointer<char*>(peb->ImageBase, importDescriptor->Name);
			Handle hModule = Loader::LoadLibrary(module);
			Assert(hModule);

			//Kernel does RT imports. Due to the PE unions, imports can't be done more than once.
			if (_stricmp(module, "mosrt.dll") != 0)
			{
				PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>(peb->ImageBase, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(peb->ImageBase, pThunkData->u1.AddressOfData);

					pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
					DebugPrintf("  Patched %s to 0x%016x\n", (char*)pImportByName->Name, pThunkData->u1.Function);
					pThunkData++;
				}
			}

			importDescriptor++;
		}
	}

	//TODO: TLS
	Loader::CrtInit((Handle)peb->ImageBase);

	//Call entry
	ProcessEntry entry = MakePointer<ProcessEntry>(peb->ImageBase, ntHeader->OptionalHeader.AddressOfEntryPoint);
	int ret = entry(peb->argc, peb->argv);

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
