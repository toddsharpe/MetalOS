#include "Kernel.h"
#include "Assert.h"

#include <sal.h>
#include <cstdint>
#include <intrin.h>
#include <ntstatus.h>
#include <windows/types.h>
#include <reactos/ioaccess.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include <kddll.h>
#include <MetalOS.Arch.h>
#include <MetalOS.Internal.h>
#include <reactos/amd64/ke.h>
#include "kd64.h"
#include "MetalOSkd.h"

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
	return (ULONG64)(LONG_PTR)Scheduler::GetCurrentThread();
}

BOOLEAN
NTAPI
KeFreezeExecution(IN PKTRAP_FRAME TrapFrame, IN PKEXCEPTION_FRAME ExceptionFrame)
{
	kernel.KePauseSystem();
	return TRUE;
}

VOID
NTAPI
KeThawExecution(IN BOOLEAN Enable)
{
	kernel.KeResumeSystem();
}

struct _KPRCB* KeGetCurrentPrcb(VOID)
{
	return &KiProcessorBlock[KeProcessor];
}

void KePrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	kernel.Printf(format, args);
	va_end(args);
}
