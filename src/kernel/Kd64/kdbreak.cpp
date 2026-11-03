/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/kdbreak.c
 * PURPOSE:         KD64 Breakpoint Support
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
 *                  Stefan Ginsberg (stefan.ginsberg@reactos.org)
 */

 /* INCLUDES ******************************************************************/

#include "core_crt/stdint.h"
#include <reactos/ntstatus.h>
#include <windows/types.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include "kddll.h"
#include <reactos/amd64/ke.h>
#include <reactos/mm.h>
#include "kd64.h"

/* FUNCTIONS *****************************************************************/

ULONG
NTAPI
KdpAddBreakpoint(IN PVOID Address)
{
	KD_BREAKPOINT_TYPE Content;
	ULONG i;
	NTSTATUS Status;

	/* Check whether we are not setting a breakpoint twice */
	for (i = 0; i < KD_BREAKPOINT_MAX; i++)
	{
		/* Check if the breakpoint is valid */
		if ((KdpBreakpointTable[i].Flags & KD_BREAKPOINT_ACTIVE) &&
			(KdpBreakpointTable[i].Address == Address))
		{
			/* Were we not able to remove it earlier? */
			if (KdpBreakpointTable[i].Flags & KD_BREAKPOINT_EXPIRED)
			{
				/* Just re-use it! */
				KdpBreakpointTable[i].Flags &= ~KD_BREAKPOINT_EXPIRED;
				return i + 1;
			}
			else
			{
				/* Fail */
				return 0;
			}
		}
	}

	/* Find a free entry */
	for (i = 0; i < KD_BREAKPOINT_MAX; i++)
	{
		if (KdpBreakpointTable[i].Flags == 0)
			break;
	}

	/* Fail if no free entry was found */
	if (i == KD_BREAKPOINT_MAX) return 0;

	/* Save the breakpoint */
	KdpBreakpointTable[i].Address = Address;

	/* If we are setting the breakpoint in user space, save the active process context */
	//if (Address < KD_HIGHEST_USER_BREAKPOINT_ADDRESS)
	//	KdpBreakpointTable[i].DirectoryTableBase = KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0];

	/* Try to save the old instruction */
	Status = KdpCopyMemoryChunks((ULONG_PTR)Address,
		&Content,
		KD_BREAKPOINT_SIZE,
		0,
		MMDBG_COPY_UNSAFE,
		NULL);
	if (NT_SUCCESS(Status))
	{
		/* Memory accessible, set the breakpoint */
		KdpBreakpointTable[i].Content = Content;
		KdpBreakpointTable[i].Flags = KD_BREAKPOINT_ACTIVE;

		/* Write the breakpoint */
		Status = KdpCopyMemoryChunks((ULONG_PTR)Address,
			&KdpBreakpointInstruction,
			KD_BREAKPOINT_SIZE,
			0,
			MMDBG_COPY_UNSAFE | MMDBG_COPY_WRITE,
			NULL);
		if (!NT_SUCCESS(Status))
		{
			/* This should never happen */
			KdpDprintf("Unable to write breakpoint to address 0x%p\n", Address);
		}
	}
	else
	{
		/* Memory is inaccessible now, setting breakpoint is deferred */
		KdpDprintf("Failed to set breakpoint at address 0x%p, adding deferred breakpoint.\n", Address);
		KdpBreakpointTable[i].Flags = KD_BREAKPOINT_ACTIVE | KD_BREAKPOINT_PENDING;
	}

	/* Return the breakpoint handle */
	return i + 1;
}

BOOLEAN
NTAPI
KdpLowWriteContent(IN ULONG BpIndex)
{
	NTSTATUS Status;

	/* Make sure that the breakpoint is actually active */
	if (KdpBreakpointTable[BpIndex].Flags & KD_BREAKPOINT_PENDING)
	{
		/* So we have a valid breakpoint, but it hasn't been used yet... */
		KdpBreakpointTable[BpIndex].Flags &= ~KD_BREAKPOINT_PENDING;
		return TRUE;
	}

	/* Is the original instruction a breakpoint anyway? */
	if (KdpBreakpointTable[BpIndex].Content == KdpBreakpointInstruction)
	{
		/* Then leave it that way... */
		return TRUE;
	}

	/* We have an active breakpoint with an instruction to bring back. Do it. */
	Status = KdpCopyMemoryChunks((ULONG_PTR)KdpBreakpointTable[BpIndex].Address,
		&KdpBreakpointTable[BpIndex].Content,
		KD_BREAKPOINT_SIZE,
		0,
		MMDBG_COPY_UNSAFE | MMDBG_COPY_WRITE,
		NULL);
	if (!NT_SUCCESS(Status))
	{
		/* Memory is inaccessible now, restoring original instruction is deferred */
		// KdpDprintf("Failed to delete breakpoint at address 0x%p\n",
		//            KdpBreakpointTable[BpIndex].Address);
		KdpBreakpointTable[BpIndex].Flags |= KD_BREAKPOINT_EXPIRED;
		return FALSE;
	}

	/* Everything went fine, return */
	return TRUE;
}

BOOLEAN
NTAPI
KdpDeleteBreakpoint(IN ULONG BpEntry)
{
	ULONG BpIndex = BpEntry - 1;

	/* Check for invalid breakpoint entry */
	if (!BpEntry || (BpEntry > KD_BREAKPOINT_MAX)) return FALSE;

	/* If the specified breakpoint table entry is not valid, then return FALSE. */
	if (!KdpBreakpointTable[BpIndex].Flags) return FALSE;

	/* Restore original data, then invalidate it and return success */
	if (KdpLowWriteContent(BpIndex)) KdpBreakpointTable[BpIndex].Flags = 0;
	return TRUE;
}

BOOLEAN
NTAPI
KdpDeleteBreakpointRange(IN PVOID Base,
	IN PVOID Limit)
{
	ULONG BpIndex;
	BOOLEAN DeletedBreakpoints;

	/* Assume no breakpoints will be deleted */
	DeletedBreakpoints = FALSE;

	/* Loop the breakpoint table */
	for (BpIndex = 0; BpIndex < KD_BREAKPOINT_MAX; BpIndex++)
	{
		/* Make sure that the breakpoint is active and matches the range. */
		if ((KdpBreakpointTable[BpIndex].Flags & KD_BREAKPOINT_ACTIVE) &&
			((KdpBreakpointTable[BpIndex].Address >= Base) &&
				(KdpBreakpointTable[BpIndex].Address <= Limit)))
		{
			/* Delete it, and remember if we succeeded at least once */
			if (KdpDeleteBreakpoint(BpIndex + 1)) DeletedBreakpoints = TRUE;
		}
	}

	/* Return whether we deleted anything */
	return DeletedBreakpoints;
}
