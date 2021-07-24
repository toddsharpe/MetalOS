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

	//KdPrint
	BOOLEAN KdpPrintString(_In_ PSTRING Output);
	BOOLEAN KdpPromptString(_In_ PSTRING PromptString, _In_ PSTRING ResponseString);

	//KdLock
	BOOLEAN KdPollBreakIn();

	//KdApi
	VOID
		NTAPI
		KdpReportExceptionStateChange(IN void* ExceptionRecord,
			IN OUT PCONTEXT Context,
			IN BOOLEAN SecondChanceException);
	VOID
		NTAPI
		KdpReadControlSpace(IN PDBGKD_MANIPULATE_STATE64 State,
			IN PSTRING Data,
			IN PCONTEXT Context);
	VOID
		NTAPI
		KdpRestoreBreakpoint(IN PDBGKD_MANIPULATE_STATE64 State,
			IN PSTRING Data,
			IN PCONTEXT Context);
	VOID
		NTAPI
		KdpReadVirtualMemory(IN PDBGKD_MANIPULATE_STATE64 State,
			IN PSTRING Data,
			IN PCONTEXT Context);
	
	VOID
		NTAPI
		KdpSetContextState(
			IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
			IN PCONTEXT Context
		);
	VOID
		NTAPI
		KdpSetCommonState(IN ULONG NewState,
			IN PCONTEXT Context,
			IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange);
	
	void
	KdpSendWaitContinue(IN ULONG PacketType,
		IN PSTRING SendHeader,
		IN PSTRING SendData OPTIONAL,
		IN OUT PCONTEXT Context);
	VOID KdpReportCommandStringStateChange(
		IN PSTRING NameString,
		IN PSTRING CommandString,
		IN OUT PCONTEXT Context
	);
	VOID
		NTAPI
		KdpSysGetVersion(IN PDBGKD_GET_VERSION64 Version);
	VOID
		NTAPI
		KdpGetVersion(IN PDBGKD_MANIPULATE_STATE64 State);


	bool IsEnabled()
	{
		return KdDebuggerEnabled;
	}

private:
	enum class State
	{
		Polling,
		OnBreakin,
		Waiting,
		Stopped
	};

	typedef struct _LDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY InLoadOrderLinks;
		LIST_ENTRY InMemoryOrderLinks;
		LIST_ENTRY InInitializationOrderLinks;
		PVOID DllBase;
		PVOID EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
		ULONG Flags;
		USHORT LoadCount;
		USHORT TlsIndex;
		union
		{
			LIST_ENTRY HashLinks;
			struct
			{
				PVOID SectionPointer;
				ULONG CheckSum;
			};
		};
		union
		{
			ULONG TimeDateStamp;
			PVOID LoadedImports;
		};
		void* EntryPointActivationContext;
		PVOID PatchInformation;
	} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

	uint32_t ThreadLoop();
	void Dispatch();

	//Dispatch functions
	void DispatchThreads();
	void DispatchGo();


	typedef void (Debugger::*CommandDispatch)();

	//Debugger state
	State m_state;
	KdDll m_dll;
	std::map<std::string, CommandDispatch> m_dispatchTable;

	//TODO
	USHORT KeProcessorLevel = 1;

	//KdData.c
	LDR_DATA_TABLE_ENTRY KernelEntry;
	LDR_DATA_TABLE_ENTRY KdComEntry;
	LIST_ENTRY PsLoadedModuleList;
	BOOLEAN KdpContextSent;

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
	//BOOLEAN KdAutoEnableOnEvent;
	//BOOLEAN KdBlockEnable;
	//BOOLEAN KdIgnoreUmExceptions;
	//BOOLEAN KdpDebuggerStructuresInitialized;
	//BOOLEAN KdEnteredDebugger;
	//ULONG KdDisableCount;

	//
	// Debugger Configuration Settings
	//
	BOOLEAN KdBreakAfterSymbolLoad;
	BOOLEAN KdPitchDebugger;
	BOOLEAN KdDebuggerNotPresent;
	BOOLEAN KdDebuggerEnabled;
	BOOLEAN KdAutoEnableOnEvent;
	BOOLEAN KdBlockEnable;
	BOOLEAN KdIgnoreUmExceptions;
	BOOLEAN KdPreviouslyEnabled;
	BOOLEAN KdpDebuggerStructuresInitialized;
	BOOLEAN KdEnteredDebugger;
	ULONG KdDisableCount;

	//
	// Buffers
	//
	CHAR KdpMessageBuffer[0x1000];
	CHAR KdpPathBuffer[0x1000];

	DBGKD_GET_VERSION64 KdVersionBlock =
	{
		0,
		0,
		DBGKD_64BIT_PROTOCOL_VERSION2,
		KD_SECONDARY_VERSION_AMD64_CONTEXT,
		DBGKD_VERS_FLAG_DATA | DBGKD_VERS_FLAG_PTR64,
		IMAGE_FILE_MACHINE_AMD64,
		PACKET_TYPE_MAX,
		0,
		0,
		DBGKD_SIMULATION_NONE,
		{0},
		0,
		0,
		0
	};
};

