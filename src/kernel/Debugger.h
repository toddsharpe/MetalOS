#pragma once

#include <windows/types.h>
#include <coreclr/list.h>
#include <reactos/ldrtypes.h>
#include <windows/winnt.h>
#include <reactos/windbgkd.h>
#include "kddll.h"
#include "kernel/Core/KThread.h"

//ReactOS::ntoskrnl\kd64\kdapi.c
//Implements the interop layer between MetalOS and KD. Simulates NT kernel being debugged
class Debugger
{
public:
	Debugger();
	void Initialize();

	void AddModule(const KModule& library);

	void DebuggerEvent(Arch::InterruptVector vector, Arch::InterruptFrame& frame);
	bool Enabled();
	bool HostPresent();

private:
	static uint32_t ThreadLoop(void* arg);
	uint32_t ThreadLoop();

	void ConvertToContext(Arch::InterruptFrame* frame, PCONTEXT context);

	LIST_ENTRY PsLoadedModuleList;
};

