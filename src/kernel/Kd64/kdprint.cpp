/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/kdprint.c
 * PURPOSE:         KD64 Trap Handler Routines
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
 *                  Stefan Ginsberg (stefan.ginsberg@reactos.org)
 */

 /* INCLUDES ******************************************************************/

#include "core_crt/stdint.h"
#include <reactos/ntstatus.h>
#include <windows/types.h>
#include <windows/winnt.h>
#include <reactos/windbgkd.h>
#include "kddll.h"

#include <coreclr/list.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/amd64/ke.h>
#include "core_crt/stdarg.h"
#include "kd64.h"
#include <reactos/ke.h>
#include "Kernel/MetalOSkd.h"

/* FUNCTIONS *****************************************************************/

BOOLEAN
NTAPI
KdpPrintString(
	PSTRING Output)
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
	DebugIo.ProcessorLevel = (USHORT)0;
	DebugIo.Processor = 0;
	DebugIo.u.PrintString.LengthOfString = Length;
	Header.Length = sizeof(DBGKD_DEBUG_IO);
	Header.Buffer = (PCHAR)&DebugIo;

	/* Build the data */
	Data.Length = Length;
	Data.Buffer = KdpMessageBuffer;

	/* Send the packet */
	KdSendPacket(PACKET_TYPE_KD_DEBUG_IO, &Header, &Data, &KdpContext);

	/* Check if the user pressed CTRL+C */
	return KdpPollBreakInWithPortLock();
}

#define _vsnprintf vsnprintf
VOID
__cdecl
KdpDprintf(
	PCCH Format,
	...)
{
	va_list ap;
	va_start(ap, Format);
	VaKdpDprintf(Format, ap);
	va_end(ap);
}

VOID
__cdecl
VaKdpDprintf(
	PCCH Format,
	va_list Args
)
{
	STRING String;
	USHORT Length;
	CHAR Buffer[100];

	/* Format the string */
	Length = (USHORT)_vsnprintf(Buffer,
		sizeof(Buffer),
		Format,
		Args);

	/* Set it up */
	String.Buffer = Buffer;
	String.Length = String.MaximumLength = Length;

	/* Send it to the debugger directly */
	KdpPrintString(&String);
}
