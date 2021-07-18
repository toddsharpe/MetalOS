#pragma once
#include "Driver.h"

#include "windbgkd.h"
#include "UartDriver.h"
#include "WindowsPE.h"
#include <kddll.h>
#include <map>

//ReactOS::ntoskrnl\kd64\kdapi.c
class Debugger
{
public:
	Debugger();
	void Initialize();

	static uint32_t ThreadLoop(void* arg) { return ((Debugger*)arg)->ThreadLoop(); };

	//KdApi
	void KdpDprintf(const char* Format, ...);
	void KdpDprintf(const char* format, va_list args);
	bool KdpPrompt(const char* Prompt, char* Response, size_t ResponseMaxSize);
	bool KdpPrompt(const std::string& Prompt, std::string& Response);

	bool IsBrokenIn()
	{
		return m_state != State::Polling;
	}

private:
	enum class State
	{
		Polling,
		OnBreakin,
		HandleCommand
	};

	uint32_t ThreadLoop();
	void Dispatch();

	//Dispatch functions
	void DispatchThreads();
	void DispatchGo();

	//KdPrint
	BOOLEAN KdpPrintString(_In_ PSTRING Output);
	BOOLEAN KdpPromptString(_In_ PCSTRING PromptString, _In_ PSTRING ResponseString);

	//KdLock
	BOOLEAN KdPollBreakIn();

	typedef void (Debugger::*CommandDispatch)();

	//Debugger state
	State m_state;
	KdDll m_dll;
	std::map<std::string, CommandDispatch> m_dispatchTable;

	//TODO
	USHORT KeProcessorLevel = 1;

	//KdData.c

	//
	// Debugger State
	//
	KD_CONTEXT KdpContext;
	//BOOLEAN KdpPortLocked;
	//KSPIN_LOCK KdpDebuggerLock;
	BOOLEAN KdpControlCPressed;
	//BOOLEAN KdpContextSent;

	//
	// Debugger Configuration Settings
	//
	//BOOLEAN KdBreakAfterSymbolLoad;
	//BOOLEAN KdPitchDebugger;
	//BOOLEAN KdDebuggerNotPresent;
	BOOLEAN KdDebuggerEnabled;
	//BOOLEAN KdAutoEnableOnEvent;
	//BOOLEAN KdBlockEnable;
	//BOOLEAN KdIgnoreUmExceptions;
	BOOLEAN KdPreviouslyEnabled;
	//BOOLEAN KdpDebuggerStructuresInitialized;
	//BOOLEAN KdEnteredDebugger;
	//ULONG KdDisableCount;

	//
	// Buffers
	//
	CHAR KdpMessageBuffer[0x1000];
};

