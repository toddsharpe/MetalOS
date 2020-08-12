#include "Runtime.h"

#include <intrin.h>
#include <cstddef>
#include <crt_string.h>

ThreadEnvironmentBlock* Runtime::GetTEB()
{
	return (ThreadEnvironmentBlock*)__readgsqword(offsetof(ThreadEnvironmentBlock, SelfPointer));
}

ProcessEnvironmentBlock* Runtime::GetPEB()
{
	ThreadEnvironmentBlock* teb = GetTEB();
	return teb->PEB;
}

Module* Runtime::GetLoadedModule(const char* name)
{
	ProcessEnvironmentBlock* peb = GetPEB();
	for (size_t i = 0; i < peb->ModuleIndex; i++)
	{
		if (stricmp(name, peb->LoadedModules[i].Name) == 0)
			return &peb->LoadedModules[i];
	}

	return nullptr;
}

extern "C" SystemCallResult GetProcessInfo(ProcessInfo& info)
{
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();
	info.Id = peb->ProcessId;
	return SystemCallResult::Success;
}

extern "C" uint64_t GetCurrentThreadId()
{
	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	return teb->ThreadId;
}

extern "C" uint32_t GetLastError()
{
	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	return teb->Error;
}

extern "C" void SetLastError(uint32_t errorCode)
{
	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	teb->Error = errorCode;
}
