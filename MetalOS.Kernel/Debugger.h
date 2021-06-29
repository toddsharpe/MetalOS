#pragma once
#include "Driver.h"

#include "windbgkd.h"
#include "UartDriver.h"
#include <kddll.h>

//ReactOS::ntoskrnl\kd64\kdapi.c
class Debugger
{
public:
	Debugger();
	void Initialize();

	static uint32_t ThreadLoop(void* arg) { return ((Debugger*)arg)->ThreadLoop(); };

	//KdPrint
	void KdPrintf(const char* Format, ...);
	BOOLEAN KdpPrintString(_In_ PSTRING Output);

private:
	enum class State
	{
		Inactive,
		Listening
	};

	uint32_t ThreadLoop();
	void Dispatch();

	State m_state;
	KdDll m_dll;

	//TODO
	USHORT KeProcessorLevel = 1;

	//KdData.c

	//
	// Debugger State
	//
	KD_CONTEXT KdpContext;
	BOOLEAN KdpPortLocked;
	//KSPIN_LOCK KdpDebuggerLock;
	BOOLEAN KdpControlCPressed;
	BOOLEAN KdpContextSent;

	//
	// Buffers
	//
	CHAR KdpMessageBuffer[0x1000];
	CHAR KdpPathBuffer[0x1000];
};

