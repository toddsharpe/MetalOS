#pragma once

typedef enum
{
	KDP_PACKET_RECEIVED = 0,
	KDP_PACKET_TIMEOUT = 1,
	KDP_PACKET_RESEND = 2
} KDP_STATUS;

Result
NTAPI
KdInitialize(
	IN ULONG ComPortNumber
);

KDP_STATUS
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

typedef Result(*OnKdInitialize)(
	IN ULONG ComPortNumber
	);
typedef KDP_STATUS(*OnKdReceivePacket)(
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

