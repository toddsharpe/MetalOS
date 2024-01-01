#include "MetalOS.h"
#include "string.h"
#include <windows/types.h>
#include <windows/winnt.h>
#include "Runtime.h"
#include "Assert.h"
#include "user/MetalOS.Types.h"
#include "Loader.h"
#include "PortableExecutable.h"

typedef int (*ProcessEntry)(int argc, char** argv);

extern "C" __declspec(dllexport) void InitProcess()//Rename: init module?
{
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();

	DebugPrintf("InitProcess\n");

	Loader::ProcessImports(peb->BaseAddress, true);
	Loader::CrtInit(peb->BaseAddress);

	//TODO: TLS

	//Call entry
	ProcessEntry entry = MakePointer<ProcessEntry>(PortableExecutable::GetEntryPoint(peb->BaseAddress));
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
