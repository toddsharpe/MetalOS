/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/kddata.c
 * PURPOSE:         Contains all global variables and settings for KD64
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
 */

 /* INCLUDES ******************************************************************/

#include <sal.h>
#include <cstdint>
#include <windows/types.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include <kernel/kddll.h>
#include <reactos/amd64/ke.h>
#include "kd64.h"

//
// Native image architecture
//
#define IMAGE_FILE_MACHINE_NATIVE IMAGE_FILE_MACHINE_AMD64

VOID NTAPI RtlpBreakWithStatusInstruction(VOID);

//
// Apply the KIPCR WDK workaround for x86 and AMD64
//
#if defined(_M_IX86) || defined(_M_AMD64)
#define KPCR KIPCR
#endif

#if defined(_M_IX86)

#define KPCR_SELF_PCR_OFFSET           FIELD_OFFSET(KPCR, SelfPcr)
#define KPCR_CURRENT_PRCB_OFFSET       FIELD_OFFSET(KPCR, Prcb)
#define KPCR_CONTAINED_PRCB_OFFSET     FIELD_OFFSET(KPCR, PrcbData)
#define KPCR_INITIAL_STACK_OFFSET      0
#define KPCR_STACK_LIMIT_OFFSET        0
#define KPRCB_PCR_PAGE_OFFSET          0
#define CBSTACK_FRAME_POINTER          Ebp

#elif defined(_M_AMD64)

#define KPCR_SELF_PCR_OFFSET           FIELD_OFFSET(KPCR, Self)
#define KPCR_CURRENT_PRCB_OFFSET       FIELD_OFFSET(KPCR, CurrentPrcb)
#define KPCR_CONTAINED_PRCB_OFFSET     FIELD_OFFSET(KPCR, Prcb)
#define KPCR_INITIAL_STACK_OFFSET      0
#define KPCR_STACK_LIMIT_OFFSET        0
#define KPRCB_PCR_PAGE_OFFSET          0
#define CBSTACK_FRAME_POINTER          Rbp

#elif defined(_M_ARM)

#define KPCR_SELF_PCR_OFFSET           0
#define KPCR_CURRENT_PRCB_OFFSET       FIELD_OFFSET(KIPCR, Prcb)
#define KPCR_CONTAINED_PRCB_OFFSET     0
#define KPCR_INITIAL_STACK_OFFSET      FIELD_OFFSET(KPCR, InitialStack)
#define KPCR_STACK_LIMIT_OFFSET        FIELD_OFFSET(KPCR, StackLimit)
#define KPRCB_PCR_PAGE_OFFSET          FIELD_OFFSET(KPRCB, PcrPage)
#define CBSTACK_FRAME_POINTER          DummyFramePointer

#else
#error Unsupported Architecture
#endif

/* GLOBALS *******************************************************************/

//
// Debugger State
//
KD_CONTEXT KdpContext;
BOOLEAN KdpPortLocked;
BOOLEAN KdpControlCPressed;
BOOLEAN KdpContextSent;

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
// Breakpoint Data
//
BREAKPOINT_ENTRY KdpBreakpointTable[KD_BREAKPOINT_MAX];
KD_BREAKPOINT_TYPE KdpBreakpointInstruction = KD_BREAKPOINT_VALUE;
BOOLEAN KdpOweBreakpoint;
BOOLEAN BreakpointsSuspended;
ULONG KdpNumInternalBreakpoints;

//
// Symbol Data
//
ULONG_PTR KdpCurrentSymbolStart, KdpCurrentSymbolEnd;

//
// Tracepoint Data
//
ULONG TraceDataBuffer[40];
ULONG TraceDataBufferPosition = 1;

//
// Buffers
//
CHAR KdpMessageBuffer[0x1000];
CHAR KdpPathBuffer[0x1000];

//
// KdPrint Buffers
//
CHAR KdPrintDefaultCircularBuffer[KD_DEFAULT_LOG_BUFFER_SIZE];
PCHAR KdPrintWritePointer = KdPrintDefaultCircularBuffer;
ULONG KdPrintRolloverCount;
PCHAR KdPrintCircularBuffer = KdPrintDefaultCircularBuffer;
ULONG KdPrintBufferSize = sizeof(KdPrintDefaultCircularBuffer);
ULONG KdPrintBufferChanges = 0;

//
// Debugger Version and Data Block
//
DBGKD_GET_VERSION64 KdVersionBlock =
{
	0,
	0,
	DBGKD_64BIT_PROTOCOL_VERSION2,
	CURRENT_KD_SECONDARY_VERSION,
#if defined(_M_AMD64) || defined(_M_ARM64)
	DBGKD_VERS_FLAG_DATA | DBGKD_VERS_FLAG_PTR64,
#else
	DBGKD_VERS_FLAG_DATA,
#endif
	IMAGE_FILE_MACHINE_NATIVE,
	PACKET_TYPE_MAX,
	0,
	0,
	DBGKD_SIMULATION_NONE,
	{0},
	0,
	0,
	0
};
