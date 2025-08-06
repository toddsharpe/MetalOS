#include "windows/types.h"
#include "windows/winnt.h"
#include "user/MetalOS.h"
#include "Lib/System.h"
#include "user/MRT/Runtime.h"
#include "user/MRT/Loader.h"
#include "Lib/StaticVector.h"
#include "Lib/Arena.h"
#include "Assert.h"

typedef int (*ProcessEntry)(int argc, char **argv);
extern "C" __declspec(dllexport) void InitProcess() // Rename: init module?
{
	ProcessEnvironmentBlock *peb = Runtime::GetPEB();

	// Headers
	const IMAGE_DOS_HEADER *dosHeader = static_cast<const PIMAGE_DOS_HEADER>(peb->ImageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	const IMAGE_NT_HEADERS64 *ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(peb->ImageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	// Imports
	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>(peb->ImageBase, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char *module = MakePointer<char *>(peb->ImageBase, importDescriptor->Name);
			Assert(module);
			HModule hModule = LoadLibrary(module);
			Assert(hModule);

			// Kernel does RT imports. Due to the PE unions, imports can't be done more than once.
			if (_stricmp(module, "mosrt.dll") != 0)
			{
				PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>(peb->ImageBase, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(peb->ImageBase, pThunkData->u1.AddressOfData);

					pThunkData->u1.Function = GetProcAddress(hModule, (char *)pImportByName->Name);
					CDebugPrintf(Runtime::IsDebug(), " Patched %s to 0x%016x\n", (char *)pImportByName->Name, pThunkData->u1.Function);
					pThunkData++;
				}
			}

			importDescriptor++;
		}
	}

	// TODO: TLS
	Loader::CrtInit(peb->ImageBase);

	//Split into argc/v
	StaticVector<const char*, 16> argv;
	StaticArena<256> arena;

	size_t start = 0;
	size_t i = 0;
	while (i < peb->CommandLine.Length)
	{
		if (peb->CommandLine[i] == ' ')
		{
			String copied = arena.Copy({&peb->CommandLine[start], (i - start)});
			argv.Add(copied.c_str());
			i++;
			start = i;
		}
		else
		{
			i++;
		}
	}
	String copied = arena.Copy({&peb->CommandLine[start], (i - start)});
	argv.Add(copied.c_str());

	// Call entry
	ProcessEntry entry = MakePointer<ProcessEntry>(peb->ImageBase, ntHeader->OptionalHeader.AddressOfEntryPoint);
	char** _argv = const_cast<char**>(argv.begin());
	const int ret = entry(argv.Count(), _argv);

	// Kill process
	ExitProcess(ret);
}

extern "C" __declspec(dllexport) void InitThread()
{
	DebugPrint("InitThread\n");

	ThreadEnvironmentBlock *teb = Runtime::GetTEB();
	teb->ThreadStart(teb->Arg);

	// Kill thread
	ExitThread(0);
}
