#include "windows/types.h"
#include "windows/winnt.h"
#include "user/MetalOS.h"
#include "Lib/System.h"
#include "user/MetalOS-RT/Runtime.h"
#include "user/MetalOS-RT/Loader.h"
#include "Lib/StaticVector.h"
#include "Lib/Math.h"
#include "WinPE.h"
#include "Assert.h"

typedef int (*ProcessEntry)(int argc, char **argv);
extern "C" __declspec(dllexport) void InitProcess() // Rename: init module?
{
	ProcessEnvironmentBlock *peb = Runtime::GetPEB();
	DebugPrintf("PEB: ImageBase=0x%016x CommandLine=%s ProcessId=%d\n", (uintptr_t)peb->ImageBase, peb->CommandLine.Data, peb->ProcessId);

	// Imports. Kernel does RT imports. Due to the PE unions, imports can't be done more than once.
	Loader::ResolveImports(peb->ImageBase, "MetalOS-RT.dll");

	// TODO: TLS
	WinPE::RunCrtInitializers(peb->ImageBase);

	//Split a copy of the command line (the kernel passes at most 127 chars) into argc/argv, ending each argument with a NUL in place of its space
	char line[128] = {};
	const size_t length = Min(peb->CommandLine.Size, sizeof(line) - 1);
	memcpy(line, peb->CommandLine.Data, length);

	//Each argument takes at least one char and a space, so every one fits, plus the NULL that ends argv
	StaticVector<char*, sizeof(line) / 2 + 1> argv;
	for (size_t i = 0; i < length; i++)
	{
		if (line[i] == ' ')
			line[i] = '\0';
		else if (i == 0 || line[i - 1] == '\0')
			argv.Add(&line[i]);
	}
	const int argc = (int)argv.Count();
	argv.Add(nullptr);

	// Call entry
	ProcessEntry entry = MakePointer<ProcessEntry>(peb->ImageBase, WinPE::GetEntryPoint(peb->ImageBase));
	const int ret = entry(argc, argv.begin());

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
