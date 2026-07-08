#pragma once

#include "user/MetalOS-RT/Runtime.h"
#include "user/MetalOS.h"
#include "x64/intrin.h"
#include "core_crt/stddef.h"

namespace Runtime
{
	ThreadEnvironmentBlock* GetTEB()
	{
		return (ThreadEnvironmentBlock*)__readgsqword(offsetof(ThreadEnvironmentBlock, SelfPointer));
	}

	ProcessEnvironmentBlock* GetPEB()
	{
		ThreadEnvironmentBlock* teb = GetTEB();
		return teb->PEB;
	}

	Module* Runtime::GetLoadedModule(const char* name)
	{
		ProcessEnvironmentBlock* peb = GetPEB();
		for (size_t i = 0; i < peb->ModuleIndex; i++)
		{
			if (_stricmp(name, peb->LoadedModules[i].Name) == 0)
				return &peb->LoadedModules[i];
		}

		return nullptr;
	}

	bool IsDebug()
	{
		return GetPEB()->Debug;
	}
}

SyscallResult GetProcessInfo(ProcessInfo* info)
{
	if (!info)
		return SyscallResult::Failed;
	
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();
	info->Id = peb->ProcessId;
	return SyscallResult::Success;
}

uint32_t GetCurrentThreadId()
{
	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	return teb->ThreadId;
}

uint32_t GetLastError()
{
	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	return teb->Error;
}

void SetLastError(uint32_t errorCode)
{
	ThreadEnvironmentBlock* teb = Runtime::GetTEB();
	teb->Error = errorCode;
}
