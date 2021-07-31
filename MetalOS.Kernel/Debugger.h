#pragma once
#include "Driver.h"

#include <windows/types.h>
#include <coreclr/list.h>
#include <reactos/ldrtypes.h>
#include <windows/winnt.h>
#include <reactos/windbgkd.h>
#include "UartDriver.h"
#include <kddll.h>
#include <map>
#include "KThread.h"

//ReactOS::ntoskrnl\kd64\kdapi.c
//Implements the interop layer between MetalOS and KD. Simulates NT kernel being debugged
class Debugger
{
public:
	Debugger();
	void Initialize();

	static uint32_t ThreadLoop(void* arg) { return ((Debugger*)arg)->ThreadLoop(); };

	void AddModule(KeLibrary& library);

	void DebuggerEvent(InterruptVector vector, PINTERRUPT_FRAME pFrame);
	void KdpDprintf(const char* format, va_list args);
	bool Enabled();

private:
	uint32_t ThreadLoop();
	void ConvertToContext(PINTERRUPT_FRAME frame, PCONTEXT context);

	LIST_ENTRY PsLoadedModuleList;
};

