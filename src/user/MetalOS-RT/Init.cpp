#include "windows/types.h"
#include "windows/winnt.h"
#include "user/MetalOS.h"
#include "Lib/System.h"
#include "user/MetalOS-RT/Runtime.h"
#include "user/MetalOS-RT/Loader.h"
#include "Lib/StaticVector.h"
#include "Lib/Arena.h"
#include "WinPE.h"
#include "Assert.h"

typedef int (*ProcessEntry)(int argc, char **argv);
extern "C" __declspec(dllexport) void InitProcess() // Rename: init module?
{
	ProcessEnvironmentBlock *peb = Runtime::GetPEB();
	DebugPrintf("PEB: ImageBase=0x%016x CommandLine=%s ProcessId=%d\n", (uintptr_t)peb->ImageBase, peb->CommandLine.c_str(), peb->ProcessId);

	// Imports. Kernel does RT imports. Due to the PE unions, imports can't be done more than once.
	Loader::ResolveImports(peb->ImageBase, "MetalOS-RT.dll");

	// TODO: TLS
	WinPE::RunCrtInitializers(peb->ImageBase);

	//Split into argc/v
	StaticVector<const char*, 16> argv;
	StaticArena<256> arena;

	size_t start = 0;
	size_t i = 0;
	while (i < peb->CommandLine.Length)
	{
		if (peb->CommandLine[i] == ' ')
		{
			const CString copied = arena.Copy({&peb->CommandLine[start], (i - start)});
			argv.Add(copied.c_str());
			i++;
			start = i;
		}
		else
		{
			i++;
		}
	}
	const CString copied = arena.Copy({&peb->CommandLine[start], (i - start)});
	argv.Add(copied.c_str());

	// Call entry
	ProcessEntry entry = MakePointer<ProcessEntry>(peb->ImageBase, WinPE::GetEntryPoint(peb->ImageBase));
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
