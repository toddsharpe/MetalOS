/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/kdlock.c
 * PURPOSE:         KD64 Port Lock and Breakin Support
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
#include <kernel/MetalOS.Arch.h>
#include <kernel/MetalOS.Internal.h>
#include <reactos/amd64/ke.h>
#include "kd64.h"

extern BOOLEAN KdDebuggerEnabled;

/* PRIVATE FUNCTIONS *********************************************************/

VOID
NTAPI
KdpPortLock(VOID)
{
	/* Acquire the lock */
	//KiAcquireSpinLock(&KdpDebuggerLock);
}

VOID
NTAPI
KdpPortUnlock(VOID)
{
	/* Release the lock */
	//KiReleaseSpinLock(&KdpDebuggerLock);
}

BOOLEAN
NTAPI
KdpPollBreakInWithPortLock(VOID)
{
	BOOLEAN DoBreak = FALSE;

	/* First make sure that KD is enabled */
	if (KdDebuggerEnabled)
	{
		/* Check if a CTRL-C is in the queue */
		if (KdpContext.KdpControlCPending)
		{
			/* Set it and prepare for break */
			DoBreak = TRUE;
			KdpContext.KdpControlCPending = FALSE;
		}
		else
		{
			/* Now get a packet */
			if (KdReceivePacket(PACKET_TYPE_KD_POLL_BREAKIN,
				NULL,
				NULL,
				NULL,
				NULL) == KdPacketReceived)
			{
				/* Successful breakin */
				DoBreak = TRUE;
			}
		}
	}

	/* Tell the caller to do a break */
	return DoBreak;
}

/* PUBLIC FUNCTIONS **********************************************************/

/*
 * @implemented
 */
BOOLEAN
NTAPI
KdPollBreakIn(VOID)
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
			/* Now get a packet */
			if (KdReceivePacket(PACKET_TYPE_KD_POLL_BREAKIN,
				NULL,
				NULL,
				NULL,
				NULL) == KdPacketReceived)
			{
				/* Successful breakin */
				DoBreak = TRUE;
				KdpControlCPressed = TRUE;
			}

			/* Let go of the port */
			KdpPortUnlock();
		}

		/* Re-enable interrupts if they were enabled previously */
		ArchRestoreFlags(flags);
	}

	/* Tell the caller to do a break */
	return DoBreak;
}
