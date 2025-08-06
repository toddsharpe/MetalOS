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

#define KD_PRINT_MAX_BYTES 512

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

BOOLEAN
NTAPI
KdpPromptString(
	PSTRING PromptString,
	PSTRING ResponseString)
{
	STRING Data, Header;
	DBGKD_DEBUG_IO DebugIo;
	ULONG Length;
	KDSTATUS Status;

	/* Copy the string to the message buffer */
	KdpMoveMemory(KdpMessageBuffer,
		PromptString->Buffer,
		PromptString->Length);

	/* Make sure we don't exceed the KD Packet size */
	Length = PromptString->Length;
	if ((sizeof(DBGKD_DEBUG_IO) + Length) > PACKET_MAX_SIZE)
	{
		/* Normalize length */
		Length = PACKET_MAX_SIZE - sizeof(DBGKD_DEBUG_IO);
	}

	/* Build the packet header */
	DebugIo.ApiNumber = DbgKdGetStringApi;
	DebugIo.ProcessorLevel = (USHORT)0;
	DebugIo.Processor = 0;
	DebugIo.u.GetString.LengthOfPromptString = Length;
	DebugIo.u.GetString.LengthOfStringRead = ResponseString->MaximumLength;
	Header.Length = sizeof(DBGKD_DEBUG_IO);
	Header.Buffer = (PCHAR)&DebugIo;

	/* Build the data */
	Data.Length = Length;
	Data.Buffer = KdpMessageBuffer;

	/* Send the packet */
	KdSendPacket(PACKET_TYPE_KD_DEBUG_IO, &Header, &Data, &KdpContext);

	/* Set the maximum lengths for the receive */
	Header.MaximumLength = sizeof(DBGKD_DEBUG_IO);
	Data.MaximumLength = sizeof(KdpMessageBuffer);

	/* Enter receive loop */
	do
	{
		/* Get our reply */
		Status = KdReceivePacket(PACKET_TYPE_KD_DEBUG_IO,
			&Header,
			&Data,
			&Length,
			&KdpContext);

		/* Return TRUE if we need to resend */
		if (Status == KdPacketNeedsResend) return TRUE;

		/* Loop until we succeed */
	} while (Status != KdPacketReceived);

	/* Don't copy back a larger response than there is room for */
	Length = Min<ULONG>(Length,
		(ULONG)ResponseString->MaximumLength);

	/* Copy back the string and return the length */
	KdpMoveMemory(ResponseString->Buffer,
		KdpMessageBuffer,
		Length);
	ResponseString->Length = (USHORT)Length;

	/* Success; we don't need to resend */
	return FALSE;
}

VOID
NTAPI
KdpCommandString(IN PSTRING NameString,
	IN PSTRING CommandString,
	IN KPROCESSOR_MODE PreviousMode,
	IN PCONTEXT ContextRecord,
	IN PKTRAP_FRAME TrapFrame,
	IN PKEXCEPTION_FRAME ExceptionFrame)
{
	BOOLEAN Enable;
	PKPRCB Prcb = KeGetCurrentPrcb();

	/* Check if we need to do anything */
	if ((PreviousMode != KernelMode) || (KdDebuggerNotPresent)) return;

	/* Enter the debugger */
	Enable = KdEnterDebugger(TrapFrame, ExceptionFrame);

	/* Save the CPU Control State and save the context */
	KiSaveProcessorControlState(&Prcb->ProcessorState);
	KdpMoveMemory(&Prcb->ProcessorState.ContextFrame,
		ContextRecord,
		sizeof(CONTEXT));

	/* Send the command string to the debugger */
	KdpReportCommandStringStateChange(NameString,
		CommandString,
		ContextRecord);

	/* Restore the processor state */
	KdpMoveMemory(ContextRecord,
		&Prcb->ProcessorState.ContextFrame,
		sizeof(CONTEXT));
	KiRestoreProcessorControlState(&Prcb->ProcessorState);

	/* Exit the debugger and return */
	KdExitDebugger(Enable);
}

VOID
NTAPI
KdpSymbol(IN PSTRING DllPath,
	IN PKD_SYMBOLS_INFO SymbolInfo,
	IN BOOLEAN Unload,
	IN KPROCESSOR_MODE PreviousMode,
	IN PCONTEXT ContextRecord,
	IN PKTRAP_FRAME TrapFrame,
	IN PKEXCEPTION_FRAME ExceptionFrame)
{
	BOOLEAN Enable;
	PKPRCB Prcb = KeGetCurrentPrcb();

	/* Check if we need to do anything */
	if ((PreviousMode != KernelMode) || (KdDebuggerNotPresent)) return;

	/* Enter the debugger */
	Enable = KdEnterDebugger(TrapFrame, ExceptionFrame);

	/* Save the CPU Control State and save the context */
	KiSaveProcessorControlState(&Prcb->ProcessorState);
	KdpMoveMemory(&Prcb->ProcessorState.ContextFrame,
		ContextRecord,
		sizeof(CONTEXT));

	/* Report the new state */
	KdpReportLoadSymbolsStateChange(DllPath,
		SymbolInfo,
		Unload,
		&Prcb->ProcessorState.ContextFrame);

	/* Restore the processor state */
	KdpMoveMemory(ContextRecord,
		&Prcb->ProcessorState.ContextFrame,
		sizeof(CONTEXT));
	KiRestoreProcessorControlState(&Prcb->ProcessorState);

	/* Exit the debugger and return */
	KdExitDebugger(Enable);
}

USHORT
NTAPI
KdpPrompt(
	PCHAR PromptString,
	USHORT PromptLength,
	PCHAR ResponseString,
	USHORT MaximumResponseLength,
	KPROCESSOR_MODE PreviousMode,
	PKTRAP_FRAME TrapFrame,
	PKEXCEPTION_FRAME ExceptionFrame)
{
	STRING PromptBuffer, ResponseBuffer;
	BOOLEAN Enable, Resend;
	PCHAR SafeResponseString;
	CHAR CapturedPrompt[KD_PRINT_MAX_BYTES];
	CHAR SafeResponseBuffer[KD_PRINT_MAX_BYTES];

	/* Normalize the lengths */
	PromptLength = Min<USHORT>(PromptLength,
		(USHORT)sizeof(CapturedPrompt));
	MaximumResponseLength = Min<USHORT>(MaximumResponseLength,
		(USHORT)sizeof(SafeResponseBuffer));

	/* Check if we need to verify the string */
	SafeResponseString = ResponseString;

	/* Setup the prompt and response buffers */
	PromptBuffer.Buffer = PromptString;
	PromptBuffer.Length = PromptBuffer.MaximumLength = PromptLength;
	ResponseBuffer.Buffer = SafeResponseString;
	ResponseBuffer.Length = 0;
	ResponseBuffer.MaximumLength = MaximumResponseLength;

	/* Log the print */
	//KdLogDbgPrint(&PromptBuffer);

	/* Enter the debugger */
	Enable = KdEnterDebugger(TrapFrame, ExceptionFrame);

	/* Enter prompt loop */
	do
	{
		/* Send the prompt and receive the response */
		Resend = KdpPromptString(&PromptBuffer, &ResponseBuffer);

		/* Loop while we need to resend */
	} while (Resend);

	/* Exit the debugger */
	KdExitDebugger(Enable);

	/* Return the number of characters received */
	return ResponseBuffer.Length;
}

NTSTATUS
NTAPI
KdpPrint(
	ULONG ComponentId,
	ULONG Level,
	PCHAR String,
	USHORT Length,
	KPROCESSOR_MODE PreviousMode,
	PKTRAP_FRAME TrapFrame,
	PKEXCEPTION_FRAME ExceptionFrame,
	PBOOLEAN Handled)
{
	NTSTATUS Status;
	BOOLEAN Enable;
	STRING OutputString;

	/* Assume failure */
	*Handled = FALSE;

	/* Normalize the length */
	Length = Min<USHORT>(Length, (USHORT)KD_PRINT_MAX_BYTES);

	/* Setup the output string */
	OutputString.Buffer = String;
	OutputString.Length = OutputString.MaximumLength = Length;

	/* Log the print */
	//KdLogDbgPrint(&OutputString);

	/* Check for a debugger */
	if (KdDebuggerNotPresent)
	{
		/* Fail */
		*Handled = TRUE;
		return STATUS_DEVICE_NOT_CONNECTED;
	}

	/* Enter the debugger */
	Enable = KdEnterDebugger(TrapFrame, ExceptionFrame);

	/* Print the string */
	if (KdpPrintString(&OutputString))
	{
		/* User pressed CTRL-C, breakpoint on return */
		Status = STATUS_BREAKPOINT;
	}
	else
	{
		/* String was printed */
		Status = STATUS_SUCCESS;
	}

	/* Exit the debugger and return */
	KdExitDebugger(Enable);
	*Handled = TRUE;
	return Status;
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
