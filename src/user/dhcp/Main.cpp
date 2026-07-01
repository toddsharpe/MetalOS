#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"
#include "user/MetalOS.h"
#include <stdio.h>
#include <string.h>

// Minimal DHCP client. Broadcasts DISCOVER, waits for OFFER, sends REQUEST, waits for ACK,
// then applies the lease (address, subnet, gateway) to interface 0.
//
// The wire format mirrors the kernel's Net/Dhcp.h. Serializer/Deserializer byte-swap the
// integer fields to/from network order; IPv4 addresses are opaque 4-byte blobs (in_addr,
// already network order) written/read with WriteBytes/ReadBytes so they aren't swapped.

static uint16_t Htons(const uint16_t v)
{
	return (uint16_t)((v >> 8) | (v << 8));
}

namespace Dhcp
{
	static const uint8_t OpRequest = 1;
	static const uint8_t HTypeEthernet = 1;
	static const uint8_t HLenEthernet = 6;
	static const uint16_t FlagBroadcast = 0x8000; // server broadcasts the reply (we have no IP yet)
	static const uint32_t MagicCookie = 0x63825363;
	static const uint16_t ServerPort = 67;
	static const uint16_t ClientPort = 68;
	static const size_t HdrSize = 240;	 // fixed BOOTP header; options follow
	static const size_t MinPacket = 300; // BOOTP minimum; shorter messages get dropped

	// Message types (option 53)
	static const uint8_t Discover = 1;
	static const uint8_t Offer = 2;
	static const uint8_t Request = 3;
	static const uint8_t Ack = 5;

	// Option codes
	static const uint8_t OptSubnet = 1;
	static const uint8_t OptRouter = 3;
	static const uint8_t OptDns = 6;
	static const uint8_t OptRequestIp = 50;
	static const uint8_t OptMsgType = 53;
	static const uint8_t OptServerId = 54;
	static const uint8_t OptParamList = 55;
	static const uint8_t OptEnd = 255;

	// The fixed BOOTP header. Integer fields are host order in memory; the serializer
	// byte-swaps them to/from network order on the wire.
	struct Header
	{
		uint8_t op;
		uint8_t htype;
		uint8_t hlen;
		uint8_t hops;
		uint32_t xid;
		uint16_t secs;
		uint16_t flags;
		in_addr ciaddr;
		in_addr yiaddr; // offered / leased address
		in_addr siaddr;
		in_addr giaddr;
		uint8_t chaddr[16]; // client hardware address (MAC + padding)
		uint8_t sname[64];
		uint8_t file[128];
		uint32_t magic;
	};
} // namespace Dhcp

// Builds a DHCP message into 'packet' (>= 576 bytes). Returns the length, padded to the
// BOOTP minimum. requestIp/serverId add options 50/54 when non-null (used for REQUEST).
static size_t BuildMessage(uint8_t *const packet, const size_t cap, const uint32_t xid, const uint8_t *const mac,
						   const uint8_t msgType, const in_addr *const requestIp, const in_addr *const serverId)
{
	//Construct the header, then serialize each field.
	const Dhcp::Header hdr = {
		.op     = Dhcp::OpRequest,
		.htype  = Dhcp::HTypeEthernet,
		.hlen   = Dhcp::HLenEthernet,
		.hops   = 0,
		.xid    = xid,
		.secs   = 0,
		.flags  = Dhcp::FlagBroadcast,
		.ciaddr = {},
		.yiaddr = {},
		.siaddr = {},
		.giaddr = {},
		.chaddr = { mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] },
		.sname  = {},
		.file   = {},
		.magic  = Dhcp::MagicCookie,
	};

	memset(packet, 0, cap);

	Serializer ser(packet, cap);
	ser.Write<uint8_t>(hdr.op);
	ser.Write<uint8_t>(hdr.htype);
	ser.Write<uint8_t>(hdr.hlen);
	ser.Write<uint8_t>(hdr.hops);
	ser.Write<uint32_t>(hdr.xid);
	ser.Write<uint16_t>(hdr.secs);
	ser.Write<uint16_t>(hdr.flags);
	ser.WriteBytes(&hdr.ciaddr.s_addr, 4);
	ser.WriteBytes(&hdr.yiaddr.s_addr, 4);
	ser.WriteBytes(&hdr.siaddr.s_addr, 4);
	ser.WriteBytes(&hdr.giaddr.s_addr, 4);
	ser.WriteBytes(hdr.chaddr, sizeof(hdr.chaddr));
	ser.WriteBytes(hdr.sname, sizeof(hdr.sname));
	ser.WriteBytes(hdr.file, sizeof(hdr.file));
	ser.Write<uint32_t>(hdr.magic);

	// Options (type-length-value) follow the fixed header.
	ser.Write<uint8_t>(Dhcp::OptMsgType);
	ser.Write<uint8_t>(1);
	ser.Write<uint8_t>(msgType);
	if (requestIp)
	{
		ser.Write<uint8_t>(Dhcp::OptRequestIp);
		ser.Write<uint8_t>(4);
		ser.WriteBytes(&requestIp->s_addr, 4);
	}
	if (serverId)
	{
		ser.Write<uint8_t>(Dhcp::OptServerId);
		ser.Write<uint8_t>(4);
		ser.WriteBytes(&serverId->s_addr, 4);
	}
	ser.Write<uint8_t>(Dhcp::OptParamList);
	ser.Write<uint8_t>(3);
	ser.Write<uint8_t>(Dhcp::OptSubnet);
	ser.Write<uint8_t>(Dhcp::OptRouter);
	ser.Write<uint8_t>(Dhcp::OptDns);
	ser.Write<uint8_t>(Dhcp::OptEnd);

	const size_t len = ser.Bytes();
	return len < Dhcp::MinPacket ? Dhcp::MinPacket : len;
}

// Deserializes the fixed BOOTP header.
static Dhcp::Header ParseHeader(const uint8_t *const msg, const size_t len)
{
	Deserializer deser(msg, len);
	Dhcp::Header h = {};
	h.op = deser.ReadU8();
	h.htype = deser.ReadU8();
	h.hlen = deser.ReadU8();
	h.hops = deser.ReadU8();
	h.xid = deser.ReadU32();
	h.secs = deser.ReadU16();
	h.flags = deser.ReadU16();
	deser.ReadBytes(&h.ciaddr.s_addr, 4);
	deser.ReadBytes(&h.yiaddr.s_addr, 4);
	deser.ReadBytes(&h.siaddr.s_addr, 4);
	deser.ReadBytes(&h.giaddr.s_addr, 4);
	return h;
}

// Returns a pointer to the value of option 'code' (options start after the fixed header),
// or nullptr. Sets *outLen to the value length.
static const uint8_t *FindOption(const uint8_t *const msg, const size_t len, const uint8_t code, uint8_t *const outLen)
{
	size_t i = Dhcp::HdrSize;
	while (i < len)
	{
		const uint8_t c = msg[i++];
		if (c == Dhcp::OptEnd)
			break;
		if (c == 0)
			continue; // pad
		if (i >= len)
			break;
		const uint8_t l = msg[i++];
		if (i + l > len)
			break;
		if (c == code)
		{
			if (outLen)
				*outLen = l;
			return msg + i;
		}
		i += l;
	}
	return nullptr;
}

// Waits for a DHCP reply of the expected message type matching our transaction id.
static bool AwaitReply(const HSocket sock, const uint8_t expectType, const uint32_t xid, uint8_t *const msg,
					   const size_t cap, size_t *const outLen)
{
	for (int attempt = 0; attempt < 4; attempt++)
	{
		size_t got = 0;
		sockaddr_in from = {};
		if (SocketRecvFrom(sock, msg, cap, &got, &from, 3000) != SyscallResult::Success || got < Dhcp::HdrSize)
			continue;

		if (ParseHeader(msg, got).xid != xid)
			continue;

		uint8_t tlen = 0;
		const uint8_t *const type = FindOption(msg, got, Dhcp::OptMsgType, &tlen);
		if (type && tlen == 1 && type[0] == expectType)
		{
			*outLen = got;
			return true;
		}
	}
	return false;
}

static void PrintIp(const char *const label, const in_addr a)
{
	const uint8_t *const b = (const uint8_t *)&a.s_addr;
	printf("%s%u.%u.%u.%u\n", label, b[0], b[1], b[2], b[3]);
}

int main(int argc, char **argv)
{
	printf("dhcp: configuring interface 0\n");

	InterfaceInfo ifaces[4] = {};
	size_t ifcount = 0;
	if (GetInterfaces(ifaces, 4, &ifcount) != SyscallResult::Success || ifcount == 0)
	{
		printf("  no interfaces found\n");
		return 1;
	}
	const InterfaceInfo &nic = ifaces[0];
	printf("  if%u mac %02x:%02x:%02x:%02x:%02x:%02x\n", nic.index, nic.mac[0], nic.mac[1], nic.mac[2], nic.mac[3],
		   nic.mac[4], nic.mac[5]);

	const HSocket sock = SocketCreate(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("  socket create failed\n");
		return 1;
	}

	const sockaddr_in local = {
		.sin_family = AF_INET,
		.sin_port   = Htons(Dhcp::ClientPort),
		.sin_addr   = {},
		.sin_zero   = {},
	};
	SocketBind(sock, &local);

	const sockaddr_in dst = {
		.sin_family = AF_INET,
		.sin_port   = Htons(Dhcp::ServerPort),
		.sin_addr   = { 0xFFFFFFFF }, // 255.255.255.255
		.sin_zero   = {},
	};

	const uint32_t xid = 0x4d4f5331; //'MOS1'
	uint8_t pkt[576] = {};
	uint8_t reply[576] = {};
	size_t replyLen = 0;

	// DISCOVER -> OFFER
	const size_t discoverLen = BuildMessage(pkt, sizeof(pkt), xid, nic.mac, Dhcp::Discover, nullptr, nullptr);
	if (SocketSendTo(sock, pkt, discoverLen, &dst) != SyscallResult::Success)
	{
		printf("  DISCOVER send failed\n");
		SocketClose(sock);
		return 1;
	}
	if (!AwaitReply(sock, Dhcp::Offer, xid, reply, sizeof(reply), &replyLen))
	{
		printf("  no OFFER received\n");
		SocketClose(sock);
		return 1;
	}

	const in_addr leased = ParseHeader(reply, replyLen).yiaddr;
	PrintIp("  OFFER ", leased);

	// REQUEST -> ACK (echo the offered address and server id back)
	uint8_t srvLen = 0;
	const uint8_t *const serverIdOpt = FindOption(reply, replyLen, Dhcp::OptServerId, &srvLen);
	in_addr serverId = {};
	if (srvLen == 4)
		memcpy(&serverId.s_addr, serverIdOpt, 4);

	const size_t requestLen =
		BuildMessage(pkt, sizeof(pkt), xid, nic.mac, Dhcp::Request, &leased, srvLen == 4 ? &serverId : nullptr);
	if (SocketSendTo(sock, pkt, requestLen, &dst) != SyscallResult::Success)
	{
		printf("  REQUEST send failed\n");
		SocketClose(sock);
		return 1;
	}
	if (!AwaitReply(sock, Dhcp::Ack, xid, reply, sizeof(reply), &replyLen))
	{
		printf("  no ACK received\n");
		SocketClose(sock);
		return 1;
	}

	// Apply the lease: address + subnet.
	const in_addr addr = ParseHeader(reply, replyLen).yiaddr;
	in_addr subnet = {};
	uint8_t snLen = 0;
	const uint8_t *const sn = FindOption(reply, replyLen, Dhcp::OptSubnet, &snLen);
	if (sn && snLen == 4)
		memcpy(&subnet.s_addr, sn, 4);
	else
		subnet.s_addr = 0x00FFFFFF; // 255.255.255.0

	if (SetInterfaceIp(nic.index, &addr, &subnet) != SyscallResult::Success)
		printf("  SetInterfaceIp failed\n");
	else
		PrintIp("  configured ", addr);

	// Default gateway (router option 3).
	uint8_t gwLen = 0;
	const uint8_t *const gw = FindOption(reply, replyLen, Dhcp::OptRouter, &gwLen);
	if (gw && gwLen >= 4)
	{
		in_addr gwAddr = {};
		memcpy(&gwAddr.s_addr, gw, 4);
		SetGateway(nic.index, &gwAddr);
		PrintIp("  gateway ", gwAddr);
	}

	SocketClose(sock);
	return 0;
}
