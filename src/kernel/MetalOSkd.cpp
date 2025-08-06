#include "Kernel.h"
#include "Assert.h"

#include "core_crt/stdint.h"
#include "x64/intrin.h"
#include <reactos/ntstatus.h>
#include <windows/types.h>
#include <reactos/ioaccess.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include "kddll.h"
#include <reactos/amd64/ke.h>
#include "Kernel/Kd64/kd64.h"
#include "Kernel/MetalOSkd.h"

extern "C" KdDll m_dll;
KdDll m_dll;
KPRCB KiProcessorBlock[KeNumberProcessors];

KDSTATUS
NTAPI
KdReceivePacket(
	IN ULONG PacketType,
	OUT PSTRING MessageHeader,
	OUT PSTRING MessageData,
	OUT PULONG DataLength,
	IN OUT PKD_CONTEXT KdContext)
{
	return m_dll.KdReceivePacket(PacketType, MessageHeader, MessageData, DataLength, KdContext);
}

VOID
NTAPI
KdSendPacket(
	IN ULONG PacketType,
	IN PSTRING MessageHeader,
	IN PSTRING MessageData,
	IN OUT PKD_CONTEXT KdContext)
{
	return m_dll.KdSendPacket(PacketType, MessageHeader, MessageData, KdContext);
}

void KeBugCheck()
{
	Assert(false);
}

ULONG64 KeGetCurrentThread()
{
	return (ULONG64)&Scheduler::GetThread();
}

bool IsValidKernelPointer(const void* address)
{
	return KeIsValid(address);
}

BOOLEAN
NTAPI
KeFreezeExecution(IN PKTRAP_FRAME TrapFrame, IN PKEXCEPTION_FRAME ExceptionFrame)
{
	KePauseSystem();
	return TRUE;
}

VOID
NTAPI
KeThawExecution(IN BOOLEAN Enable)
{
	KeResumeSystem();
}

struct _KPRCB* KeGetCurrentPrcb(VOID)
{
	return &KiProcessorBlock[KeProcessor];
}

void KePrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	Printf(format, args);
	va_end(args);
}
