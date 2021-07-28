#pragma once

typedef ULONG KDSTATUS;
#define KdPacketReceived     0
#define KdPacketTimedOut     1
#define KdPacketNeedsResend  2

NTSTATUS
NTAPI
KdInitialize(
	IN ULONG ComPortNumber
);

KDSTATUS
NTAPI
KdReceivePacket(
	IN ULONG PacketType,
	OUT PSTRING MessageHeader,
	OUT PSTRING MessageData,
	OUT PULONG DataLength,
	IN OUT PKD_CONTEXT Context
);

VOID
NTAPI
KdSendPacket(
	IN ULONG PacketType,
	IN PSTRING MessageHeader,
	IN PSTRING MessageData,
	IN OUT PKD_CONTEXT Context
);

typedef NTSTATUS(*OnKdInitialize)(
	IN ULONG ComPortNumber
	);
typedef KDSTATUS(*OnKdReceivePacket)(
	IN ULONG PacketType,
	OUT PSTRING MessageHeader,
	OUT PSTRING MessageData,
	OUT PULONG DataLength,
	IN OUT PKD_CONTEXT Context
	);
typedef VOID(*OnKdSendPacket)(
	IN ULONG PacketType,
	IN PSTRING MessageHeader,
	IN PSTRING MessageData,
	IN OUT PKD_CONTEXT Context
	);

struct KdDll
{
	OnKdInitialize KdInitialize;
	OnKdReceivePacket KdReceivePacket;
	OnKdSendPacket KdSendPacket;
};

