#pragma once

#define KeProcessor 0
#define KeNumberProcessors 1

void KeBugCheck();
ULONG64 KeGetCurrentThread();
bool IsValidKernelPointer(const void* address);

BOOLEAN
NTAPI
KeFreezeExecution(IN PKTRAP_FRAME TrapFrame,
	IN PKEXCEPTION_FRAME ExceptionFrame);

VOID
NTAPI
KeThawExecution(IN BOOLEAN Enable);

struct _KPRCB* KeGetCurrentPrcb(VOID);

extern KPRCB KiProcessorBlock[KeNumberProcessors];

void KePrintf(const char* format, ...);
