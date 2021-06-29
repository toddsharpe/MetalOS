/*
 * COPYRIGHT:       GPL, see COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            drivers/base/kddll/kdcom.c
 * PURPOSE:         COM port functions for the kernel debugger.
 * PROGRAMMER:      Timo Kreuzer (timo.kreuzer@reactos.org)
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <WindowsPE.h>
#include "ntstatus.h"
#include "basetsd.h"
#include "windbgkd.h"
#include <MetalOS.Internal.h>

#include "kddll.h"
#include "kdcom.h"
#include "cportlib.h"

 /* Serial debug connection */
#if defined(SARCH_PC98)
#define DEFAULT_DEBUG_PORT      2 /* COM2 */
#define DEFAULT_DEBUG_COM1_IRQ  4
#define DEFAULT_DEBUG_COM2_IRQ  5
#define DEFAULT_DEBUG_BAUD_RATE 9600
#define DEFAULT_BAUD_RATE       9600
#else
#define DEFAULT_DEBUG_PORT      2 /* COM2 */
#define DEFAULT_DEBUG_COM1_IRQ  4
#define DEFAULT_DEBUG_COM2_IRQ  3
#define DEFAULT_DEBUG_BAUD_RATE 115200
#define DEFAULT_BAUD_RATE       19200
#endif

#if defined(_M_IX86) || defined(_M_AMD64)
#if defined(SARCH_PC98)
const ULONG BaseArray[] = { 0, 0x30, 0x238 };
#else
const ULONG BaseArray[] = { 0, 0x3F8, 0x2F8, 0x3E8, 0x2E8 };
#endif
#elif defined(_M_PPC)
const ULONG BaseArray[] = { 0, 0x800003F8 };
#elif defined(_M_MIPS)
const ULONG BaseArray[] = { 0, 0x80006000, 0x80007000 };
#elif defined(_M_ARM)
const ULONG BaseArray[] = { 0, 0xF1012000 };
#else
#error Unknown architecture
#endif

#define MAX_COM_PORTS   (sizeof(BaseArray) / sizeof(BaseArray[0]) - 1)

/* GLOBALS ********************************************************************/

CPPORT KdComPort;
ULONG  KdComPortIrq = 0; // Not used at the moment.
#ifdef KDDEBUG
CPPORT KdDebugComPort;
#endif

NTSTATUS
NTAPI
KdpPortInitialize(IN ULONG ComPortNumber,
	IN ULONG ComPortBaudRate)
{
	NTSTATUS Status;

	KDDBGPRINT("KdpPortInitialize, Port = COM%ld\n", ComPortNumber);

	Status = CpInitialize(&KdComPort,
		(PUCHAR)UlongToPtr(BaseArray[ComPortNumber]),
		ComPortBaudRate);
	if (!NT_SUCCESS(Status))
	{
		return STATUS_INVALID_PARAMETER;
	}
	else
	{
		return STATUS_SUCCESS;
	}
}

/******************************************************************************
 * \name KdInitialize
 * \brief Initialization.
 * \param [opt] LoaderBlock Pointer to the Loader parameter block. Can be NULL.
 * \return Status
 */
Result
NTAPI
KdInitialize(IN ULONG ComPortNumber)
{
	ULONG ComPortBaudRate = DEFAULT_DEBUG_BAUD_RATE;


	KDDBGPRINT("KdDebuggerInitialize0\n");

	/* Initialize the port */
	NTSTATUS status = KdpPortInitialize(ComPortNumber, ComPortBaudRate);
	if (status == STATUS_SUCCESS)
		return Result::Success;
	else
		return Result::Failed;
}

VOID
NTAPI
KdpSendByte(IN UCHAR Byte)
{
	/* Send the byte */
	CpPutByte(&KdComPort, Byte);
}

KDP_STATUS
NTAPI
KdpPollByte(OUT PUCHAR OutByte)
{
	USHORT Status;

	/* Poll the byte */
	Status = CpGetByte(&KdComPort, OutByte, FALSE, FALSE);
	switch (Status)
	{
	case CP_GET_SUCCESS:
		return KDP_PACKET_RECEIVED;

	case CP_GET_NODATA:
		return KDP_PACKET_TIMEOUT;

	case CP_GET_ERROR:
	default:
		return KDP_PACKET_RESEND;
	}
}

KDP_STATUS
NTAPI
KdpReceiveByte(OUT PUCHAR OutByte)
{
	USHORT Status;

	/* Get the byte */
	Status = CpGetByte(&KdComPort, OutByte, TRUE, FALSE);
	switch (Status)
	{
	case CP_GET_SUCCESS:
		return KDP_PACKET_RECEIVED;

	case CP_GET_NODATA:
		return KDP_PACKET_TIMEOUT;

	case CP_GET_ERROR:
	default:
		return KDP_PACKET_RESEND;
	}
}

KDP_STATUS
NTAPI
KdpPollBreakIn(VOID)
{
	KDP_STATUS KdStatus;
	UCHAR Byte;

	KdStatus = KdpPollByte(&Byte);
	if ((KdStatus == KDP_PACKET_RECEIVED) && (Byte == BREAKIN_PACKET_BYTE))
	{
		return KDP_PACKET_RECEIVED;
	}
	return KDP_PACKET_TIMEOUT;
}

/* EOF */
