#include "string.h"
#include <windows/types.h>
#include <windows/winnt.h>
#include "Runtime.h"
#include <shared/MetalOS.Types.h>
#include <user/MetalOS.h>

typedef int (*ProcessEntry)(int argc, char** argv);
typedef void (*CrtInitializer)();

extern PIMAGE_SECTION_HEADER GetPESection(Handle imageBase, const char* name);

template<typename T>
constexpr T MakePointer(const void* base, size_t offset = 0)
{
	return reinterpret_cast<T>((char*)base + offset);
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
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>((void*)baseAddress, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePointer<char*>((void*)baseAddress, importDescriptor->Name);
			Handle hModule = LoadLibrary(module);
			if (hModule == nullptr)
				ExitProcess(-1);

			if (stricmp(module, "mosrt.dll") != 0)
			{
				PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>((void*)baseAddress, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>((void*)baseAddress, pThunkData->u1.AddressOfData);

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
