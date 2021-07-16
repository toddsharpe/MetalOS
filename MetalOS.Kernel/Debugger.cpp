#include "Debugger.h"
#include "Main.h"
#include <WindowsPE.h>
#include <windbgkd.h>
#include "Loader.h"
#include "PortableExecutable.h"
#include <algorithm>

#define KdpMoveMemory memcpy
#define KdpZeroMemory(dst, len) memset(dst, 0, len)

#define VER_PRODUCTBUILD                    3790

Debugger::Debugger() :
	m_state(State::Polling),
	KdDebuggerEnabled(true),
	KdDebuggerNotPresent(false)
{
	static uint16_t KernelName[16] = L"moskrnl.exe";
	static uint16_t KdComName[16] = L"kdcom.dll";
	
	ULONG NtBuildNumber = VER_PRODUCTBUILD;

	/* Fill out the KD Version Block */
	KdVersionBlock.MajorVersion = DBGKD_MAJOR_EFI;
	KdVersionBlock.MinorVersion = 7601;// (USHORT)(NtBuildNumber & 0xFFFF);
	
	/* Set protocol limits */
	KdVersionBlock.MaxStateChange = DbgKdMaximumStateChange -
		DbgKdMinimumStateChange;
	KdVersionBlock.MaxManipulate = DbgKdMaximumManipulate -
		DbgKdMinimumManipulate;
	KdVersionBlock.Unused[0] = 0;

	KdVersionBlock.KernBase = KernelBaseAddress;
	KdVersionBlock.PsLoadedModuleList = (ULONG64)(LONG_PTR)&PsLoadedModuleList;

	InitializeListHead(&PsLoadedModuleList);

	const KeLibrary* hKernel = kernel.KeGetModule("moskrnl.exe");

	KernelEntry.DllBase = (void*)hKernel->Handle;
	KernelEntry.EntryPoint = (void*)((uintptr_t)hKernel->Handle + PortableExecutable::GetEntryPoint(hKernel->Handle));
	KernelEntry.SizeOfImage = PortableExecutable::GetSizeOfImage(hKernel->Handle);
	KernelEntry.FullDllName = { (USHORT)wcslen(KernelName), 0, KernelName };
	KernelEntry.BaseDllName = { (USHORT)wcslen(KernelName), 0, KernelName };
	KernelEntry.LoadCount = 1;

	InsertTailList(&PsLoadedModuleList, &KernelEntry.InLoadOrderLinks);

	PLIST_ENTRY NextEntry;
	PLDR_DATA_TABLE_ENTRY LdrEntry;
	for (NextEntry = PsLoadedModuleList.Flink;
		NextEntry != &PsLoadedModuleList;
		NextEntry = NextEntry->Flink)
	{
		LdrEntry = CONTAINING_RECORD(NextEntry,
			LDR_DATA_TABLE_ENTRY,
			InLoadOrderLinks);
		
		char buffer[256] = { 0 };
		wcstombs(buffer, LdrEntry->BaseDllName.Buffer, 256);
		kernel.Printf("Dll: %s\n", &buffer);
	}
}

void Debugger::Initialize()
{
	Handle kddll = kernel.KeLoadLibrary("kdcom.dll");

	//Load pointers
	m_dll.KdInitialize = (OnKdInitialize)PortableExecutable::GetProcAddress(kddll, "KdInitialize");
	m_dll.KdReceivePacket = (OnKdReceivePacket)PortableExecutable::GetProcAddress(kddll, "KdReceivePacket");
	m_dll.KdSendPacket = (OnKdSendPacket)PortableExecutable::GetProcAddress(kddll, "KdSendPacket");

	kernel.Printf("KdInitialize loaded at 0x%016x\n", m_dll.KdInitialize);
	kernel.Printf("KdReceivePacket loaded at 0x%016x\n", m_dll.KdReceivePacket);
	kernel.Printf("KdSendPacket loaded at 0x%016x\n", m_dll.KdSendPacket);

	Result result = m_dll.KdInitialize(2);
	kernel.Printf("KdInitialized %d\n", result);

	KThread* thread = kernel.CreateKernelThread(Debugger::ThreadLoop, this);
	thread->SetName("DebuggerLoop");


	KdpDprintf("MetalOS::KdCom initialized!\n");
}

uint32_t Debugger::ThreadLoop()
{
	while (true)
	{
		this->Dispatch();
	}

	return 0;
}

void Debugger::Dispatch()
{
	switch (m_state)
	{
	case State::Polling:
		if (KdPollBreakIn())
		{
			kernel.KePauseSystem();
			m_state = State::OnBreakin;
		}
		else
		{
			kernel.Sleep(50);
		}
		break;

	case State::OnBreakin:
	{
		KdpDprintf("MetalOS::Broken in\n");

		m_state = State::Waiting;
	}
	break;


	case State::Waiting:
	{
		kernel.Printf("Waiting\n");
		
		char name[32] = "Test Name\n";
		char command[32] = "Test Command\n";

		/*
		STRING n = { 0 };
		n.Buffer = name;
		n.Length = strlen(name);

		STRING c = { 0 };
		c.Buffer = command;
		c.Length = strlen(command);
		CONTEXT ctx = { 0 };
		KdpReportCommandStringStateChange(&n, &c, &ctx);
		kernel.Printf("Name: %s Command: %s\n", &name, &command);
		*/

		CONTEXT ctx = { 0 };
		KdpReportExceptionStateChange(nullptr, &ctx, false);

		//char name[32] = "Command?\n";
		//char command[32];
		//KdpZeroMemory(command, sizeof(command) / sizeof(char));

		//STRING n = { 0 };
		//n.Buffer = name;
		//n.MaximumLength = 32;
		//n.Length = strlen(name);

		//STRING c = { 0 };
		//c.Buffer = command;
		//c.Length = 32;
		//c.MaximumLength = 32;
		//CONTEXT ctx = { 0 };

		//KdpPromptString(&n, &c);
		//kernel.Printf("Response: %s\n", &command);
		m_state = State::Stopped;
	}
	break;

	case State::Stopped:
		kernel.Printf("Stopped\n");
		m_state = State::Polling;
		kernel.KeResumeSystem();
		break;
	}
}

VOID
NTAPI
Debugger::KdpSetCommonState(IN ULONG NewState,
	IN PCONTEXT Context,
	IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange)
{
	ULONG InstructionCount;
	BOOLEAN HadBreakpoints;

	/* Setup common stuff available for all CPU architectures */
	WaitStateChange->NewState = NewState;
	WaitStateChange->ProcessorLevel = 1;
	WaitStateChange->Processor = (USHORT)0;
	WaitStateChange->NumberProcessors = (ULONG)1;
	WaitStateChange->Thread = (ULONG64)(LONG_PTR)kernel.GetCurrentThread();
	WaitStateChange->ProgramCounter = (ULONG64)(LONG_PTR)_ReturnAddress();

	/* Zero out the entire Control Report */
	KdpZeroMemory(&WaitStateChange->AnyControlReport,
		sizeof(DBGKD_ANY_CONTROL_REPORT));

	WaitStateChange->ControlReport.InstructionCount = DBGKD_MAXSTREAM;
	KdpMoveMemory(WaitStateChange->ControlReport.InstructionStream, (void*)WaitStateChange->ProgramCounter, DBGKD_MAXSTREAM);
	WaitStateChange->ControlReport.ReportFlags = REPORT_INCLUDES_SEGS | REPORT_STANDARD_CS;
}

VOID
NTAPI
Debugger::KdpSetContextState(IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
	IN PCONTEXT Context)
{
	/* Copy i386 specific debug registers */
	//WaitStateChange->ControlReport.Dr6 = Prcb->ProcessorState.SpecialRegisters.
	//	KernelDr6;
	//WaitStateChange->ControlReport.Dr7 = Prcb->ProcessorState.SpecialRegisters.
	//	KernelDr7;

	/* Copy i386 specific segments */
	WaitStateChange->ControlReport.SegCs = (USHORT)Context->SegCs;
	WaitStateChange->ControlReport.SegDs = (USHORT)Context->SegDs;
	WaitStateChange->ControlReport.SegEs = (USHORT)Context->SegEs;
	WaitStateChange->ControlReport.SegFs = (USHORT)Context->SegFs;

	/* Copy EFlags */
	WaitStateChange->ControlReport.EFlags = Context->EFlags;

	/* Set Report Flags */
	WaitStateChange->ControlReport.ReportFlags = REPORT_INCLUDES_SEGS;
	//if (WaitStateChange->ControlReport.SegCs == KGDT64_R0_CODE)
	//{
	//	WaitStateChange->ControlReport.ReportFlags |= REPORT_STANDARD_CS;
	//}
}

#define ANSI_NULL ((CHAR)0)     // winnt
VOID
NTAPI
Debugger::KdpReportCommandStringStateChange(IN PSTRING NameString,
	IN PSTRING CommandString,
	IN OUT PCONTEXT Context)
{
	STRING Header, Data;
	DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;
	ULONG Length, ActualLength, TotalLength;

	/* Start wait loop */
	do
	{
		/* Build the architecture common parts of the message */
		KdpSetCommonState(DbgKdCommandStringStateChange,
			Context,
			&WaitStateChange);

		/* Set the context */
		//KdpSetContextState(&WaitStateChange, Context);

		/* Clear the command string structure */
		KdpZeroMemory(&WaitStateChange.u.CommandString,
			sizeof(DBGKD_COMMAND_STRING));

		/* Normalize name string to max */
		Length = std::min(128 - 1, (int)NameString->Length);

		/* Copy it to the message buffer */
		KdpMoveMemory(KdpMessageBuffer,
			(void*)NameString->Buffer,
			Length);
		ActualLength = Length;
		//KdpCopyMemoryChunks((ULONG_PTR)NameString->Buffer,
		//	KdpMessageBuffer,
		//	Length,
		//	0,
		//	MMDBG_COPY_UNSAFE,
		//	&ActualLength);

		/* Null terminate and calculate the total length */
		TotalLength = ActualLength;
		KdpMessageBuffer[TotalLength++] = ANSI_NULL;

		/* Check if the command string is too long */
		Length = CommandString->Length;
		if (Length > (PACKET_MAX_SIZE -
			sizeof(DBGKD_ANY_WAIT_STATE_CHANGE) - TotalLength))
		{
			/* Use maximum possible size */
			Length = (PACKET_MAX_SIZE -
				sizeof(DBGKD_ANY_WAIT_STATE_CHANGE) - TotalLength);
		}

		/* Copy it to the message buffer */
		KdpMoveMemory(KdpMessageBuffer + TotalLength,
			(void*)CommandString->Buffer,
			Length);
		ActualLength = Length;
		//KdpCopyMemoryChunks((ULONG_PTR)CommandString->Buffer,
		//	KdpMessageBuffer + TotalLength,
		//	Length,
		//	0,
		//	MMDBG_COPY_UNSAFE,
		//	&ActualLength);

		/* Null terminate and calculate the total length */
		TotalLength += ActualLength;
		KdpMessageBuffer[TotalLength++] = ANSI_NULL;

		/* Now set up the header and the data */
		Header.Length = sizeof(DBGKD_ANY_WAIT_STATE_CHANGE);
		Header.Buffer = (PCHAR)&WaitStateChange;
		Data.Length = (USHORT)TotalLength;
		Data.Buffer = KdpMessageBuffer;

		/* Send State Change packet and wait for a reply */
		KdpSendWaitContinue(PACKET_TYPE_KD_STATE_CHANGE64,
			&Header,
			&Data,
			Context);
	} while (false /*Status == ContinueProcessorReselected*/);
}

#define STATUS_BREAKPOINT                ((NTSTATUS)0x80000003L)    // winnt

VOID
NTAPI
Debugger::KdpReportExceptionStateChange(IN void* ExceptionRecord,
	IN OUT PCONTEXT Context,
	IN BOOLEAN SecondChanceException)
{
	STRING Header, Data;
	DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange = { 0 };

	/* Build the architecture common parts of the message */
	KdpSetCommonState(DbgKdExceptionStateChange, Context, &WaitStateChange);

	///* Just copy it directly, no need to convert */
	//KdpMoveMemory(&WaitStateChange.u.Exception.ExceptionRecord,
	//	ExceptionRecord,
	//	sizeof(EXCEPTION_RECORD));
	WaitStateChange.u.Exception.ExceptionRecord.ExceptionAddress = (uintptr_t)WaitStateChange.ProgramCounter;
	WaitStateChange.u.Exception.ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;

	/* Set the First Chance flag */
	WaitStateChange.u.Exception.FirstChance = !SecondChanceException;
	WaitStateChange.u.Exception.ExceptionRecord.NumberParameters = 3;
	WaitStateChange.u.Exception.ExceptionRecord.ExceptionInformation[0] = 0;
	WaitStateChange.u.Exception.ExceptionRecord.ExceptionInformation[1] = (ULONG64)(LONG_PTR)kernel.GetCurrentThread();
	WaitStateChange.u.Exception.ExceptionRecord.ExceptionInformation[2] = 0;

	/* Now finish creating the structure */
	KdpSetContextState(&WaitStateChange, Context);

	/* Setup the actual header to send to KD */
	Header.Length = sizeof(DBGKD_ANY_WAIT_STATE_CHANGE);
	Header.Buffer = (PCHAR)&WaitStateChange;

	/* Setup the trace data */
	char b[32] = "test";
	Data.Buffer = b;
	Data.Length = strlen(b);

	kernel.Printf("PC: 0x%016x\n", WaitStateChange.ProgramCounter);
	kernel.Printf("Thread: 0x%016x\n", WaitStateChange.Thread);
	kernel.Printf("PsLoadedModuleList: 0x%016x\n", &PsLoadedModuleList);

	/* Send State Change packet and wait for a reply */
	KdpSendWaitContinue(PACKET_TYPE_KD_STATE_CHANGE64,
		&Header,
		&Data,
		Context);
}

VOID
NTAPI
Debugger::KdpSysGetVersion(IN PDBGKD_GET_VERSION64 Version)
{
	/* Copy the version block */
	KdpMoveMemory(Version,
		&KdVersionBlock,
		sizeof(DBGKD_GET_VERSION64));
}

VOID
NTAPI
Debugger::KdpGetVersion(IN PDBGKD_MANIPULATE_STATE64 State)
{
	STRING Header;

	kernel.Printf("KdpGetVersion\n");

	/* Fill out the header */
	Header.Length = sizeof(DBGKD_MANIPULATE_STATE64);
	Header.Buffer = (PCHAR)State;

	/* Get the version block */
	KdpSysGetVersion(&State->u.GetVersion64);

	/* Fill out the state */
	State->ApiNumber = DbgKdGetVersionApi;
	State->ReturnStatus = STATUS_SUCCESS;

	/* Send the packet */
	m_dll.KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
		&Header,
		NULL,
		&KdpContext);
}

VOID
NTAPI
Debugger::KdpReadVirtualMemory(IN PDBGKD_MANIPULATE_STATE64 State,
	IN PSTRING Data,
	IN PCONTEXT Context)
{
	PDBGKD_READ_MEMORY64 ReadMemory = &State->u.ReadMemory;
	STRING Header;
	ULONG Length = ReadMemory->TransferCount;

	kernel.Printf("KdpReadVirtualMemory A: 0x%016x S: 0x%x\n", ReadMemory->TargetBaseAddress, Length);

	/* Setup the header */
	Header.Length = sizeof(DBGKD_MANIPULATE_STATE64);
	Header.Buffer = (PCHAR)State;
	Assert(Data->Length == 0);

	/* Validate length */
	if (Length > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64)))
	{
		/* Overflow, set it to maximum possible */
		Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64);
	}

	if (ReadMemory->TargetBaseAddress >= 0xfffff78000000000 || ReadMemory->TargetBaseAddress < KernelStart)
	{
		KdpZeroMemory(Data->Buffer, Length);
		State->ReturnStatus = STATUS_UNSUCCESSFUL;
	}
	else
	{
		KdpMoveMemory(Data->Buffer, (void*)ReadMemory->TargetBaseAddress, Length);
		State->ReturnStatus = STATUS_SUCCESS;
	}

	//KdpMoveMemory((void*)ReadMemory->TargetBaseAddress,
	//	Data->Buffer,
	//	Length);

	///* Do the read */
	//State->ReturnStatus = KdpCopyMemoryChunks(ReadMemory->TargetBaseAddress,
	//	Data->Buffer,
	//	Length,
	//	0,
	//	MMDBG_COPY_UNSAFE,
	//	&Length);


	/* Return the actual length read */
	ReadMemory->ActualBytesRead = Length;
	Data->Length = (USHORT)Length;

	/* Send the packet */
	m_dll.KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
		&Header,
		Data,
		&KdpContext);
}

VOID
NTAPI
Debugger::KdpRestoreBreakpoint(IN PDBGKD_MANIPULATE_STATE64 State,
	IN PSTRING Data,
	IN PCONTEXT Context)
{
	PDBGKD_RESTORE_BREAKPOINT RestoreBp = &State->u.RestoreBreakPoint;
	STRING Header;

	/* Fill out the header */
	Header.Length = sizeof(DBGKD_MANIPULATE_STATE64);
	Header.Buffer = (PCHAR)State;
	Assert(Data->Length == 0);

	kernel.Printf("KdpRestoreBreakpoint: 0x%x\n", RestoreBp->BreakPointHandle);

	/* Get the version block */
	if (true /*KdpDeleteBreakpoint(RestoreBp->BreakPointHandle)*/)
	{
		/* We're all good */
		State->ReturnStatus = STATUS_SUCCESS;
	}
	else
	{
		/* We failed */
		State->ReturnStatus = STATUS_UNSUCCESSFUL;
	}

	/* Send the packet */
	m_dll.KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
		&Header,
		NULL,
		&KdpContext);
}

VOID
NTAPI
Debugger::KdpReadControlSpace(IN PDBGKD_MANIPULATE_STATE64 State,
	IN PSTRING Data,
	IN PCONTEXT Context)
{
	PDBGKD_READ_MEMORY64 ReadMemory = &State->u.ReadMemory;
	STRING Header;
	ULONG Length;

	/* Setup the header */
	Header.Length = sizeof(DBGKD_MANIPULATE_STATE64);
	Header.Buffer = (PCHAR)State;
	Assert(Data->Length == 0);

	/* Check the length requested */
	Length = ReadMemory->TransferCount;
	if (Length > (PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64)))
	{
		/* Use maximum allowed */
		Length = PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64);
	}
	kernel.Printf("KdpReadControlSpace A:0x%016x S:0x%x\n",
		ReadMemory->TargetBaseAddress, Length);
	/* Call the internal routine */
	//State->ReturnStatus = KdpSysReadControlSpace(State->Processor,
	//	ReadMemory->TargetBaseAddress,
	//	Data->Buffer,
	//	Length,
	//	&Length);


	/* Return the actual length read */
	ReadMemory->ActualBytesRead = Length;
	Data->Length = (USHORT)Length;

	/* Send the reply */
	m_dll.KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
		&Header,
		Data,
		&KdpContext);
}

void Debugger::KdpSendWaitContinue(IN ULONG PacketType, IN PSTRING SendHeader, IN PSTRING SendData OPTIONAL, IN OUT PCONTEXT Context)
{
	STRING Data, Header;
	DBGKD_MANIPULATE_STATE64 ManipulateState;
	ULONG Length;
	KDP_STATUS RecvCode;

	/* Setup the Manipulate State structure */
	Header.MaximumLength = sizeof(DBGKD_MANIPULATE_STATE64);
	Header.Buffer = (PCHAR)&ManipulateState;
	Data.MaximumLength = sizeof(KdpMessageBuffer);
	Data.Buffer = KdpMessageBuffer;

	/*
	 * Reset the context state to ensure the debugger has received
	 * the current context before it sets it.
	 */
	KdpContextSent = FALSE;

SendPacket:
	/* Send the Packet */
	m_dll.KdSendPacket(PacketType, SendHeader, SendData, &KdpContext);

	/* If the debugger isn't present anymore, just return success */
	//if (KdDebuggerNotPresent) return;

	/* Main processing Loop */
	for (;;)
	{
		/* Receive Loop */
		do
		{
			/* Wait to get a reply to our packet */
			RecvCode = m_dll.KdReceivePacket(PACKET_TYPE_KD_STATE_MANIPULATE,
				&Header,
				&Data,
				&Length,
				&KdpContext);

			/* If we got a resend request, do it */
			if (RecvCode == KDP_STATUS::KDP_PACKET_RESEND) goto SendPacket;
		} while (RecvCode == KDP_STATUS::KDP_PACKET_TIMEOUT);

		/* Now check what API we got */
		switch (ManipulateState.ApiNumber)
		{
			case DbgKdReadVirtualMemoryApi:

				/* Read virtual memory */
				KdpReadVirtualMemory(&ManipulateState, &Data, Context);
				break;

			//case DbgKdWriteVirtualMemoryApi:

			//	/* Write virtual memory */
			//	KdpWriteVirtualMemory(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdGetContextApi:

			//	/* Get the current context */
			//	KdpGetContext(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdSetContextApi:

			//	/* Set a new context */
			//	KdpSetContext(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdWriteBreakPointApi:

			//	/* Write the breakpoint */
			//	KdpWriteBreakpoint(&ManipulateState, &Data, Context);
			//	break;

			case DbgKdRestoreBreakPointApi:

				/* Restore the breakpoint */
				KdpRestoreBreakpoint(&ManipulateState, &Data, Context);
				break;

			case DbgKdContinueApi:

				/* Simply continue */
				return;// NT_SUCCESS(ManipulateState.u.Continue.ContinueStatus);

			case DbgKdReadControlSpaceApi:

				/* Read control space */
				KdpReadControlSpace(&ManipulateState, &Data, Context);
				break;

			//case DbgKdWriteControlSpaceApi:

			//	/* Write control space */
			//	KdpWriteControlSpace(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdReadIoSpaceApi:

			//	/* Read I/O Space */
			//	KdpReadIoSpace(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdWriteIoSpaceApi:

			//	/* Write I/O Space */
			//	KdpWriteIoSpace(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdRebootApi:

			//	/* Reboot the system */
			//	HalReturnToFirmware(HalRebootRoutine);
			//	break;

			case DbgKdContinueApi2:
				kernel.Printf("DbgKdContinueApi2\n");
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
				/* Check if caller reports success */
				if (NT_SUCCESS(ManipulateState.u.Continue2.ContinueStatus))
				{
					/* Update the state */
					//KdpGetStateChange(&ManipulateState, Context);
					return;// ContinueSuccess;
				}
				else
				{
					/* Return an error */
					return;// ContinueError;
				}

			//case DbgKdReadPhysicalMemoryApi:

			//	/* Read  physical memory */
			//	KdpReadPhysicalMemory(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdWritePhysicalMemoryApi:

			//	/* Write  physical memory */
			//	KdpWritePhysicalMemory(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdQuerySpecialCallsApi:
			//case DbgKdSetSpecialCallApi:
			//case DbgKdClearSpecialCallsApi:

			//	/* TODO */
			//	KdpDprintf("Special Call support is unimplemented!\n");
			//	KdpNotSupported(&ManipulateState);
			//	break;

			//case DbgKdSetInternalBreakPointApi:
			//case DbgKdGetInternalBreakPointApi:

			//	/* TODO */
			//	KdpDprintf("Internal Breakpoint support is unimplemented!\n");
			//	KdpNotSupported(&ManipulateState);
			//	break;

			//case DbgKdReadIoSpaceExtendedApi:

			//	/* Read I/O Space */
			//	KdpReadIoSpaceExtended(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdWriteIoSpaceExtendedApi:

			//	/* Write I/O Space */
			//	KdpWriteIoSpaceExtended(&ManipulateState, &Data, Context);
			//	break;

			case DbgKdGetVersionApi:

				/* Get version data */
				KdpGetVersion(&ManipulateState);
				break;

			//case DbgKdWriteBreakPointExApi:

			//	/* Write the breakpoint and check if it failed */
			//	if (!NT_SUCCESS(KdpWriteBreakPointEx(&ManipulateState,
			//		&Data,
			//		Context)))
			//	{
			//		/* Return an error */
			//		return ContinueError;
			//	}
			//	break;

			//case DbgKdRestoreBreakPointExApi:

			//	/* Restore the breakpoint */
			//	KdpRestoreBreakPointEx(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdCauseBugCheckApi:

			//	/* Crash the system */
			//	KdpCauseBugCheck(&ManipulateState);
			//	break;

			//case DbgKdSwitchProcessor:

			//	/* TODO */
			//	KdpDprintf("Processor Switch support is unimplemented!\n");
			//	KdpNotSupported(&ManipulateState);
			//	break;

			//case DbgKdPageInApi:

			//	/* TODO */
			//	KdpDprintf("Page-In support is unimplemented!\n");
			//	KdpNotSupported(&ManipulateState);
			//	break;

			//case DbgKdReadMachineSpecificRegister:

			//	/* Read from the specified MSR */
			//	KdpReadMachineSpecificRegister(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdWriteMachineSpecificRegister:

			//	/* Write to the specified MSR */
			//	KdpWriteMachineSpecificRegister(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdSearchMemoryApi:

			//	/* Search memory */
			//	KdpSearchMemory(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdGetBusDataApi:

			//	/* Read from the bus */
			//	KdpGetBusData(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdSetBusDataApi:

			//	/* Write to the bus */
			//	KdpSetBusData(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdCheckLowMemoryApi:

			//	/* Check for memory corruption in the lower 4 GB */
			//	KdpCheckLowMemory(&ManipulateState);
			//	break;

			case DbgKdClearAllInternalBreakpointsApi:

				/* Just clear the counter */
				//KdpNumInternalBreakpoints = 0;
				break;

			//case DbgKdFillMemoryApi:

			//	/* Fill memory */
			//	KdpFillMemory(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdQueryMemoryApi:

			//	/* Query memory */
			//	KdpQueryMemory(&ManipulateState, Context);
			//	break;

			//case DbgKdSwitchPartition:

			//	/* TODO */
			//	KdpDprintf("Partition Switch support is unimplemented!\n");
			//	KdpNotSupported(&ManipulateState);
			//	break;

			//case DbgKdWriteCustomBreakpointApi:

			//	/* Write the customized breakpoint */
			//	KdpWriteCustomBreakpoint(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdGetContextExApi:

			//	/* Extended Context Get */
			//	KdpGetContextEx(&ManipulateState, &Data, Context);
			//	break;

			//case DbgKdSetContextExApi:

			//	/* Extended Context Set */
			//	KdpSetContextEx(&ManipulateState, &Data, Context);
			//	break;

				/* Unsupported Messages */
		default:

			/* Send warning */
			KdpDprintf("Received Unrecognized API 0x%lx\n", ManipulateState.ApiNumber);

			/* Setup an empty message, with failure */
			Data.Length = 0;
			ManipulateState.ReturnStatus = STATUS_UNSUCCESSFUL;

			/* Send it */
			m_dll.KdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
				&Header,
				&Data,
				&KdpContext);
			break;
		}
	}
}

#define _vsnprintf vsnprintf
void Debugger::KdpDprintf(const char* Format, ...)
{
	STRING String;
	USHORT Length;
	va_list ap;
	CHAR Buffer[100];

	/* Format the string */
	va_start(ap, Format);
	Length = (USHORT)_vsnprintf(Buffer,
		sizeof(Buffer),
		Format,
		ap);
	va_end(ap);

	/* Set it up */
	String.Buffer = Buffer;
	String.Length = String.MaximumLength = Length;

	/* Send it to the debugger directly */
	KdpPrintString(&String);
}

void Debugger::KdpDprintf(const char* format, va_list args)
{
	char buffer[255];

	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';

	STRING String;
	String.Buffer = buffer;
	String.Length = strlen(buffer);

	KdpPrintString(&String);
}

BOOLEAN Debugger::KdpPrintString(PSTRING Output)
{
	STRING Data, Header;
	DBGKD_DEBUG_IO DebugIo;
	USHORT Length;

	/* Copy the string */
	KdpMoveMemory(KdpMessageBuffer,
		Output->Buffer,
		Output->Length);

	/* Make sure we don't exceed the KD Packet size */
	Length = Output->Length;
	if ((sizeof(DBGKD_DEBUG_IO) + Length) > PACKET_MAX_SIZE)
	{
		/* Normalize length */
		Length = PACKET_MAX_SIZE - sizeof(DBGKD_DEBUG_IO);
	}

	/* Build the packet header */
	DebugIo.ApiNumber = DbgKdPrintStringApi;
	DebugIo.ProcessorLevel = (USHORT)KeProcessorLevel;
	//DebugIo.Processor = KeGetCurrentPrcb()->Number;
	DebugIo.Processor = 0;
	DebugIo.u.PrintString.LengthOfString = Length;
	Header.Length = sizeof(DBGKD_DEBUG_IO);
	Header.Buffer = (PCHAR)&DebugIo;

	/* Build the data */
	Data.Length = Length;
	Data.Buffer = KdpMessageBuffer;

	/* Send the packet */
	m_dll.KdSendPacket(PACKET_TYPE_KD_DEBUG_IO, &Header, &Data, &KdpContext);

	/* Check if the user pressed CTRL+C */
	return KdPollBreakIn();
}


BOOLEAN Debugger::KdPollBreakIn()
{
	BOOLEAN DoBreak = FALSE, Enable;

	/* First make sure that KD is enabled */
	if (KdDebuggerEnabled)
	{
		/* Disable interrupts */
		cpu_flags_t flags = ArchDisableInterrupts();

		/* Check if a CTRL-C is in the queue */
		if (KdpContext.KdpControlCPending)
		{
			/* Set it and prepare for break */
			KdpControlCPressed = TRUE;
			DoBreak = TRUE;
			KdpContext.KdpControlCPending = FALSE;
		}
		else
		{
			/* Try to acquire the lock */
			//if (KeTryToAcquireSpinLockAtDpcLevel(&KdpDebuggerLock))
			//{
				/* Now get a packet */
			if (m_dll.KdReceivePacket(PACKET_TYPE_KD_POLL_BREAKIN,
				NULL,
				NULL,
				NULL,
				NULL) == KDP_STATUS::KDP_PACKET_RECEIVED)
			{
				/* Successful breakin */
				DoBreak = TRUE;
				KdpControlCPressed = TRUE;
			}

			/* Let go of the port */
			//KdpPortUnlock();
		//}
		}

		/* Re-enable interrupts if they were enabled previously */
		//if (Enable) _enable();
		ArchRestoreFlags(flags);
	}

	/* Tell the caller to do a break */
	return DoBreak;
}
