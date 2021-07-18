#include "Debugger.h"
#include "Main.h"
#include <WindowsPE.h>
#include <windbgkd.h>
#include "Loader.h"
#include "PortableExecutable.h"
#include <algorithm>
#include <sstream>

#define KdpMoveMemory memcpy
#define KdpZeroMemory(dst, len) memset(dst, 0, len)

#define VER_PRODUCTBUILD                    3790

Debugger::Debugger() :
	m_state(State::Polling),
	KdDebuggerEnabled(true),
	m_dispatchTable()
{
	m_dispatchTable["!threads"] = &Debugger::DispatchThreads;
	m_dispatchTable["g"] = &Debugger::DispatchGo;
}

void Debugger::Initialize()
{
	//Handle file = kernel.CreateFile("kdcom.dll", GenericAccess::Read);
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

		m_state = State::HandleCommand;
	}
	break;


	case State::HandleCommand:
	{
		std::string response;
		KdpPrompt("Command:\n", response);

		const auto& it = m_dispatchTable.find(response);
		if (it != m_dispatchTable.end())
		{
			(this->*it->second)();
		}
		else
		{
			KdpDprintf("Unknown command: %s\n", response.c_str());
		}

	}
	break;
	}
}

void Debugger::DispatchThreads()
{
	KdpDprintf("Threads Ready: 0x%d\n", kernel.m_scheduler->m_readyQueue.size());
	for (const auto & item : kernel.m_scheduler->m_readyQueue)
	{
		item->Display();
	}

	KdpDprintf("Threads Sleeping: 0x%d\n", kernel.m_scheduler->m_sleepQueue.size());
	for (const auto& item : kernel.m_scheduler->m_sleepQueue)
	{
		item->Display();
	}
}

void Debugger::DispatchGo()
{
	cpu_flags_t flags = ArchDisableInterrupts();
	kernel.KeResumeSystem();
	this->m_state = State::Polling;
	ArchRestoreFlags(flags);
}

bool Debugger::KdpPrompt(const std::string& Prompt, std::string& Response)
{
	CSTRING PromptBuffer = { Prompt.length(), 0, Prompt.c_str() };

	//TODO: do we need this buffer here?
	char responseBuffer[128];
	STRING ResponseBuffer;
	ResponseBuffer.Buffer = responseBuffer;
	ResponseBuffer.MaximumLength = sizeof(responseBuffer) / sizeof(char);

	bool result = KdpPromptString(&PromptBuffer, &ResponseBuffer);
	Response = responseBuffer;
	return result;
}

bool Debugger::KdpPrompt(const char* Prompt, char* Response, size_t ResponseMaxSize)
{
	CSTRING PromptBuffer = { strlen(Prompt), 0, Prompt };

	STRING ResponseBuffer;
	ResponseBuffer.Buffer = Response;
	ResponseBuffer.MaximumLength = ResponseMaxSize;

	return KdpPromptString(&PromptBuffer, &ResponseBuffer);
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

BOOLEAN Debugger::KdpPrintString(_In_ PSTRING Output)
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

BOOLEAN Debugger::KdpPromptString(_In_ PCSTRING PromptString, _In_ PSTRING ResponseString)
{
	STRING Data, Header;
	DBGKD_DEBUG_IO DebugIo;
	ULONG Length;
	KDP_STATUS Status;

	/* Copy the string to the message buffer */
	KdpMoveMemory(KdpMessageBuffer,
		PromptString->Buffer,
		PromptString->Length);

	/* Make sure we don't exceed the KD Packet size */
	Length = PromptString->Length;
	if ((sizeof(DBGKD_DEBUG_IO) + Length) > PACKET_MAX_SIZE)
	{
		/* Normalize length */
		Length = PACKET_MAX_SIZE - sizeof(DBGKD_DEBUG_IO);
	}

	/* Build the packet header */
	DebugIo.ApiNumber = DbgKdGetStringApi;
	DebugIo.ProcessorLevel = (USHORT)KeProcessorLevel;
	//DebugIo.Processor = KeGetCurrentPrcb()->Number;
	DebugIo.u.GetString.LengthOfPromptString = Length;
	DebugIo.u.GetString.LengthOfStringRead = ResponseString->MaximumLength;
	Header.Length = sizeof(DBGKD_DEBUG_IO);
	Header.Buffer = (PCHAR)&DebugIo;

	/* Build the data */
	Data.Length = Length;
	Data.Buffer = KdpMessageBuffer;

	/* Send the packet */
	m_dll.KdSendPacket(PACKET_TYPE_KD_DEBUG_IO, &Header, &Data, &KdpContext);

	/* Set the maximum lengths for the receive */
	Header.MaximumLength = sizeof(DBGKD_DEBUG_IO);
	Data.MaximumLength = sizeof(KdpMessageBuffer);

	/* Enter receive loop */
	do
	{
		/* Get our reply */
		Status = m_dll.KdReceivePacket(PACKET_TYPE_KD_DEBUG_IO,
			&Header,
			&Data,
			&Length,
			&KdpContext);

		/* Return TRUE if we need to resend */
		if (Status == KDP_STATUS::KDP_PACKET_RESEND) return TRUE;

		/* Loop until we succeed */
	} while (Status != KDP_STATUS::KDP_PACKET_RECEIVED);

	/* Don't copy back a larger response than there is room for */
	Length = std::min(Length,
		(ULONG)ResponseString->MaximumLength);

	/* Copy back the string and return the length */
	KdpMoveMemory(ResponseString->Buffer,
		KdpMessageBuffer,
		Length);
	ResponseString->Length = (USHORT)Length;

	/* Success; we don't need to resend */
	return FALSE;
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
