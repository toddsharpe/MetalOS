/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/kdtrap.c
 * PURPOSE:         KD64 Trap Handlers
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
 *                  Stefan Ginsberg (stefan.ginsberg@reactos.org)
 */

 /* INCLUDES ******************************************************************/

#include "core_crt/stdint.h"
#include <reactos/ntstatus.h>
#include <windows/types.h>
#include <windows/winnt.h>
#include <reactos/kdtypes.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include <reactos/amd64/ke.h>
#include <reactos/ke.h>
#include "kddll.h"
#include "kd64.h"
#include "kernel/Debug/MetalOSkd.h"

/* FUNCTIONS *****************************************************************/

BOOLEAN
NTAPI
KdpReport(IN PKTRAP_FRAME TrapFrame,
	IN PKEXCEPTION_FRAME ExceptionFrame,
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PCONTEXT ContextRecord,
	IN KPROCESSOR_MODE PreviousMode,
	IN BOOLEAN SecondChanceException)
{
	BOOLEAN Enable, Handled;
	PKPRCB Prcb;
	NTSTATUS ExceptionCode;

	/*
	 * Determine whether to pass the exception to the debugger.
	 * First, check if this is a "debug exception", meaning breakpoint
	 * (including debug service), single step and assertion failure exceptions.
	 */
	ExceptionCode = ExceptionRecord->ExceptionCode;
	if ((ExceptionCode == STATUS_BREAKPOINT) ||
		(ExceptionCode == STATUS_SINGLE_STEP) ||
		(ExceptionCode == STATUS_ASSERTION_FAILURE))
	{
		/* This is a debug exception; we always pass them to the debugger */
	}
	else
	{
		/* Let the exception handler, if any, try to handle it */
		return FALSE;
	}

	/* Enter the debugger */
	Enable = KdEnterDebugger(TrapFrame, ExceptionFrame);

	/*
	 * Get the KPRCB and save the CPU Control State manually instead of
	 * using KiSaveProcessorState, since we already have a valid CONTEXT.
	 */
	Prcb = KeGetCurrentPrcb();
	KiSaveProcessorControlState(&Prcb->ProcessorState);
	KdpMoveMemory(&Prcb->ProcessorState.ContextFrame,
		ContextRecord,
		sizeof(CONTEXT));

	/* Report the new state */
	Handled = KdpReportExceptionStateChange(ExceptionRecord,
		ContextRecord,
		SecondChanceException);

	/* Now restore the processor state, manually again. */
	KdpMoveMemory(ContextRecord,
		&Prcb->ProcessorState.ContextFrame,
		sizeof(CONTEXT));
	KiRestoreProcessorControlState(&Prcb->ProcessorState);

	/* Exit the debugger */
	KdExitDebugger(Enable);
	return Handled;
}

BOOLEAN
NTAPI
KdpTrap(IN PKTRAP_FRAME TrapFrame,
	IN PKEXCEPTION_FRAME ExceptionFrame,
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PCONTEXT ContextRecord,
	IN KPROCESSOR_MODE PreviousMode,
	IN BOOLEAN SecondChanceException)
{
	BOOLEAN Handled;

	/* Call the worker routine */
	Handled = KdpReport(TrapFrame,
		ExceptionFrame,
		ExceptionRecord,
		ContextRecord,
		PreviousMode,
		SecondChanceException);

	/* Return TRUE or FALSE to caller */
	return Handled;
}
