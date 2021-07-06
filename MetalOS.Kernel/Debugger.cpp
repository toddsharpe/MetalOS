#include "Debugger.h"
#include "Main.h"
#include <windbgkd.h>
#include "Loader.h"
#include "PortableExecutable.h"

#define KdpMoveMemory memcpy

Debugger::Debugger() :
	m_state(State::Inactive)
{

}

void Debugger::Initialize()
{
	//kernel.CreateKernelThread(Debugger::ThreadLoop, this);

	//Handle file = kernel.CreateFile("kdcom.dll", GenericAccess::Read);
	Handle kddll = Loader::LoadKernelLibrary("kdcom.dll");
	kernel.Printf("kddll loaded at 0x%016x\n", kddll);

	//Load pointers
	m_dll.KdInitialize = (OnKdInitialize)PortableExecutable::GetProcAddress(kddll, "KdInitialize");
	m_dll.KdReceivePacket = (OnKdReceivePacket)PortableExecutable::GetProcAddress(kddll, "KdReceivePacket");
	m_dll.KdSendPacket = (OnKdSendPacket)PortableExecutable::GetProcAddress(kddll, "KdSendPacket");

	kernel.Printf("KdInitialize loaded at 0x%016x\n", m_dll.KdInitialize);
	kernel.Printf("KdReceivePacket loaded at 0x%016x\n", m_dll.KdReceivePacket);
	kernel.Printf("KdSendPacket loaded at 0x%016x\n", m_dll.KdSendPacket);

	Result result = m_dll.KdInitialize(2);
	kernel.Printf("KdInitialized %d\n", result);

	KdPrintf("MetalOS::KdCom initialized!\n");
}

uint32_t Debugger::ThreadLoop()
{
	while (true)
	{
		this->Dispatch();

		kernel.Sleep(100);
	}

	return 0;
}

void Debugger::Dispatch()
{
	KD_PACKET packet;

}

void Debugger::KdPrintf(const char* Format, ...)
{
	STRING String;
	USHORT Length;
	va_list ap;
	CHAR Buffer[100];

	/* Format the string */
	va_start(ap, Format);
	Length = vsprintf(Buffer, Format, ap);
	va_end(ap);

	/* Set it up */
	String.Buffer = Buffer;
	String.Length = String.MaximumLength = Length;

	/* Send it to the debugger directly */
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
	//return KdpPollBreakInWithPortLock();
	return FALSE;
}

