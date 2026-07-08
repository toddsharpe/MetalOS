#pragma once

#include "user/MetalOS.h"

namespace Runtime
{
	ThreadEnvironmentBlock* GetTEB();
	ProcessEnvironmentBlock* GetPEB();
	Module* GetLoadedModule(const char* name);
	bool IsDebug();
}

extern "C"
{
	SyscallResult GetProcessInfo(ProcessInfo* info);
	uint32_t GetCurrentThreadId();
	uint32_t GetLastError();
	void SetLastError(uint32_t errorCode);
}
