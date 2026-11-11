#pragma once

#include "kernel/Arch.h"
#include "kernel/Types.h"

//The layer between MetalOS and KD, which sees an NT kernel being debugged (after ReactOS ntoskrnl/kd64/kdapi.c); its state lives in Debugger.cpp
namespace Debugger
{
	void Initialize();

	void AddModule(const KModule& library);

	void DebuggerEvent(Arch::InterruptVector vector, Arch::InterruptFrame& frame);
	bool Enabled();
	bool HostPresent();
}
