/*
 * COPYRIGHT:       GPL, see COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            drivers/base/kddll/kddll.h
 * PURPOSE:         Base definitions for the kernel debugger.
 * PROGRAMMER:      Timo Kreuzer (timo.kreuzer@reactos.org)
 */

#pragma once

 // #define KDDEBUG /* uncomment to enable debugging this dll */

#ifndef KDDEBUG
#define KDDBGPRINT(...)
#else
extern ULONG KdpDbgPrint(const char* Format, ...);
#define KDDBGPRINT KdpDbgPrint
#endif

typedef enum
{
	KDP_PACKET_RECEIVED = 0,
	KDP_PACKET_TIMEOUT = 1,
	KDP_PACKET_RESEND = 2
} KDP_STATUS;

VOID
NTAPI
KdpSendBuffer(
	IN PVOID Buffer,
	IN ULONG Size);

KDP_STATUS
NTAPI
KdpReceiveBuffer(
	OUT PVOID Buffer,
	IN  ULONG Size);

KDP_STATUS
NTAPI
KdpReceivePacketLeader(
	OUT PULONG PacketLeader);

VOID
NTAPI
KdpSendByte(IN UCHAR Byte);

KDP_STATUS
NTAPI
KdpPollByte(OUT PUCHAR OutByte);

KDP_STATUS
NTAPI
KdpReceiveByte(OUT PUCHAR OutByte);

KDP_STATUS
NTAPI
KdpPollBreakIn(VOID);
