#pragma once

#include <windows/types.h>
#include <coreclr/list.h>
#include <reactos/ldrtypes.h>
#include <windows/winnt.h>
#include <reactos/windbgkd.h>
#include "kddll.h"
#include "KThread.h"

//ReactOS::ntoskrnl\kd64\kdapi.c
//Implements the interop layer between MetalOS and KD. Simulates NT kernel being debugged
class Debugger
{
public:
	Debugger();
	void Initialize();

	void AddModule(const KModule& library);

	void DebuggerEvent(Arch::InterruptVector vector, Arch::InterruptFrame& frame);
	void KdpDprintf(const char* format, ...);
	void KdpDprintf(const char* format, va_list args);
	bool Enabled();

private:
	static uint32_t ThreadLoop(void* arg);
	uint32_t ThreadLoop();

	void ConvertToContext(Arch::InterruptFrame* frame, PCONTEXT context);

	StaticArena<Arch::PageSize> m_arena;

	LIST_ENTRY PsLoadedModuleList;
};

