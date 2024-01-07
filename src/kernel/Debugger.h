#pragma once

#include <windows/types.h>
#include <coreclr/list.h>
#include <reactos/ldrtypes.h>
#include <windows/winnt.h>
#include <reactos/windbgkd.h>
#include "kddll.h"
#include <map>
#include "KThread.h"

//ReactOS::ntoskrnl\kd64\kdapi.c
//Implements the interop layer between MetalOS and KD. Simulates NT kernel being debugged
class Debugger
{
public:
	Debugger();
	void Initialize();

	void AddModule(const KeModule& library);

	void DebuggerEvent(X64_INTERRUPT_VECTOR vector, X64_INTERRUPT_FRAME* frame);
	void KdpDprintf(const char* format, va_list args);
	bool Enabled();

private:
	static size_t ThreadLoop(void* arg);
	size_t ThreadLoop();

	void ConvertToContext(X64_INTERRUPT_FRAME* frame, PCONTEXT context);

	LIST_ENTRY PsLoadedModuleList;
};

