/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/include/internal/kd64.h
 * PURPOSE:         Internal header for the KD64 Library
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
 */

#pragma once

#include "reactos/amd64/ketypes.h"
#include "reactos/ketypes.h"
#include "reactos/amd64/ke.h"

//
// Maximum supported number of breakpoints
//
#define KD_BREAKPOINT_MAX   32

//
// Highest limit starting which we consider that breakpoint addresses
// are either in system space, or in user space but inside shared DLLs.
//
// I'm wondering whether this can be computed using MmHighestUserAddress
// or whether there is already some #define somewhere else...
// See http://www.drdobbs.com/windows/faster-dll-load-load/184416918
// and http://www.drdobbs.com/rebasing-win32-dlls/184416272
// for a tentative explanation.
//
#define KD_HIGHEST_USER_BREAKPOINT_ADDRESS  (PVOID)0x60000000  // MmHighestUserAddress

//
// Breakpoint Status Flags
//
#define KD_BREAKPOINT_ACTIVE    0x01
#define KD_BREAKPOINT_PENDING   0x02
#define KD_BREAKPOINT_EXPIRED   0x08

//
// Structure for Breakpoints
//
typedef struct _BREAKPOINT_ENTRY
{
    ULONG Flags;
    ULONG_PTR DirectoryTableBase;
    PVOID Address;
    KD_BREAKPOINT_TYPE Content;
} BREAKPOINT_ENTRY, * PBREAKPOINT_ENTRY;

//
// Debug Trap Handlers
//
BOOLEAN
NTAPI
KdpTrap(
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChanceException
);

//
// Port Locking
//
VOID
NTAPI
KdpPortUnlock(
    VOID
);

BOOLEAN
NTAPI
KdpPollBreakInWithPortLock(
    VOID
);

BOOLEAN
NTAPI
KdPollBreakIn(
    VOID
);

//
// Debugger Enter and Exit
//
BOOLEAN
NTAPI
KdEnterDebugger(
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
);

VOID
NTAPI
KdExitDebugger(
    IN BOOLEAN Enable
);

//
// State Change Notifications
//
BOOLEAN
NTAPI
KdpReportExceptionStateChange(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT Context,
    IN BOOLEAN SecondChanceException
);

//
// Breakpoint Support
//
ULONG
NTAPI
KdpAddBreakpoint(
    IN PVOID Address
);

BOOLEAN
NTAPI
KdpDeleteBreakpoint(
    IN ULONG BpEntry
);

BOOLEAN
NTAPI
KdpDeleteBreakpointRange(
    IN PVOID Base,
    IN PVOID Limit
);

//
// Safe memory read & write Support
//
NTSTATUS
NTAPI
KdpCopyMemoryChunks(
    ULONG64 Address,
    PVOID Buffer,
    ULONG TotalSize,
    ULONG ChunkSize,
    ULONG Flags,
    PULONG ActualSize
);

//
// Internal memory handling routines for KD isolation
//
VOID
NTAPI
KdpMoveMemory(
    PVOID Destination,
    PVOID Source,
    SIZE_T Length
);

VOID
NTAPI
KdpZeroMemory(
    PVOID Destination,
    SIZE_T Length
);

//
// Low Level Support Routines for the KD API
//

//
// Version
//
VOID
NTAPI
KdpSysGetVersion(
    IN PDBGKD_GET_VERSION64 Version
);

//
// Context
//
VOID
NTAPI
KdpGetStateChange(
    IN PDBGKD_MANIPULATE_STATE64 State,
    IN PCONTEXT Context
);

VOID
NTAPI
KdpSetContextState(
    IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PCONTEXT Context
);

//
// MSR
//
NTSTATUS
NTAPI
KdpSysReadMsr(
    IN ULONG Msr,
    OUT PLARGE_INTEGER MsrValue
);

NTSTATUS
NTAPI
KdpSysWriteMsr(
    IN ULONG Msr,
    IN PLARGE_INTEGER MsrValue
);

//
// Bus
//
NTSTATUS
NTAPI
KdpSysReadBusData(
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG ActualLength
);

NTSTATUS
NTAPI
KdpSysWriteBusData(
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG ActualLength
);

//
// Control Space
//
NTSTATUS
NTAPI
KdpSysReadControlSpace(
    IN ULONG Processor,
    IN ULONG64 BaseAddress,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG ActualLength
);

NTSTATUS
NTAPI
KdpSysWriteControlSpace(
    IN ULONG Processor,
    IN ULONG64 BaseAddress,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG ActualLength
);

//
// I/O Space
//
NTSTATUS
NTAPI
KdpSysReadIoSpace(
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 IoAddress,
    IN PVOID DataValue,
    IN ULONG DataSize,
    OUT PULONG ActualDataSize
);

NTSTATUS
NTAPI
KdpSysWriteIoSpace(
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 IoAddress,
    IN PVOID DataValue,
    IN ULONG DataSize,
    OUT PULONG ActualDataSize
);

//
// Low Memory
//
NTSTATUS
NTAPI
KdpSysCheckLowMemory(
    IN ULONG Flags
);

//
// Internal routine for sending strings directly to the debugger
//
VOID
__cdecl
KdpDprintf(
    PCCH Format,
    ...
);

VOID
__cdecl
VaKdpDprintf(
    PCCH Format,
    va_list Args
);

//
// Global KD Data
//
extern BOOLEAN KdDebuggerEnabled;

extern DBGKD_GET_VERSION64 KdVersionBlock;
extern KD_CONTEXT KdpContext;
extern BOOLEAN KdpPortLocked;
extern BOOLEAN KdpContextSent;

extern CHAR KdpMessageBuffer[0x1000];
extern BREAKPOINT_ENTRY KdpBreakpointTable[KD_BREAKPOINT_MAX];
extern KD_BREAKPOINT_TYPE KdpBreakpointInstruction;
extern ULONG KdpNumInternalBreakpoints;
extern ULONG TraceDataBuffer[40];
extern ULONG TraceDataBufferPosition;

extern BOOLEAN KdDebuggerNotPresent;
