/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/kddata.c
 * PURPOSE:         Contains all global variables and settings for KD64
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
 */

 /* INCLUDES ******************************************************************/

#include "core_crt/stdint.h"
#include <windows/types.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include "kddll.h"
#include <reactos/amd64/ke.h>
#include "kd64.h"

//
// Native image architecture
//
#define IMAGE_FILE_MACHINE_NATIVE IMAGE_FILE_MACHINE_AMD64

/* GLOBALS *******************************************************************/

//
// Debugger State
//
KD_CONTEXT KdpContext;
BOOLEAN KdpPortLocked;
BOOLEAN KdpContextSent;

//
// Debugger Configuration Settings
//
BOOLEAN KdDebuggerNotPresent;
BOOLEAN KdDebuggerEnabled;

//
// Breakpoint Data
//
BREAKPOINT_ENTRY KdpBreakpointTable[KD_BREAKPOINT_MAX];
KD_BREAKPOINT_TYPE KdpBreakpointInstruction = KD_BREAKPOINT_VALUE;
ULONG KdpNumInternalBreakpoints;

//
// Tracepoint Data
//
ULONG TraceDataBuffer[40];
ULONG TraceDataBufferPosition = 1;

//
// Buffers
//
CHAR KdpMessageBuffer[0x1000];

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
