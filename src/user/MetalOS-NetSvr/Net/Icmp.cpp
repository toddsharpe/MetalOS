#include "Net/NetIf.h"
#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"

namespace Net::Icmp
{
	static void ReceiveEchoRequest(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < echo_hdr_size)
			return;

		Deserializer deser(packet.buffer(), packet.length());
		const echo_hdr_t hdr =
		{
			.id = deser.ReadU16(),
			.seq_num = deser.ReadU16(),
		};
		packet.mask(echo_hdr_size);
		packet.proto = proto_t::EchoRequest;

		const NetLayer& l2 = get_l2(net_if.l2);

		//Allocate space
		const buffer_t buffer = alloc_tx(buffer_size);
		if (!buffer)
			return;

		Packet reply(buffer);
		reply.mask(l2.HeaderSize);
		reply.mask(ipv4_hdr_size);
		reply.mask(icmp_hdr_size);

		//Reflect id, seq num, and payload
		Serializer ser(reply.buffer(), reply.length());
		ser.Write(hdr.id);
		ser.Write(hdr.seq_num);
		ser.WriteBytes(deser.Peek(), deser.Remaining());
		reply.trim(ser.Bytes());

		//Send response
		reply.proto = proto_t::EchoReply;
		reply.dst = packet.src;
		reply.src = packet.dst;
		if (!Icmp::Send(net_if, reply))
			free_tx(buffer);
	}

	//What follows the unused word is the head of the datagram that was refused: its IPv4 header, then its UDP header
	static void ReceiveDestUnreachable(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < dest_unreachable_size + ipv4_hdr_size + udp_hdr_size)
		{
			net_if.ipv4.rx_dropped++;
			return;
		}

		Deserializer deser(packet.buffer(), packet.length());
		deser.Skip(dest_unreachable_size);
		const uint8_t ihl_version = deser.ReadU8();
		deser.Skip(8);
		const uint8_t proto = deser.ReadU8();
		deser.Skip(2);
		const ipv4_addr_t src = { .addr = deser.ReadU32() };
		const ipv4_addr_t dst = { .addr = deser.ReadU32() };
		if (proto != ip_proto_udp || ihl_version != 0x45)
			return;

		const uint16_t src_port = deser.ReadU16();
		const uint16_t dst_port = deser.ReadU16();

		//The refused datagram was ours, so its source is the local socket and its destination the peer that refused it
		Socket::ReceiveUnreachable(net_if, { src, src_port }, { dst, dst_port });
	}

	bool Receive(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < icmp_hdr_size || !checksum_valid(packet.buffer(), packet.length()))
		{
			net_if.ipv4.rx_dropped++;
			return false;
		}

		Deserializer deser(packet.buffer(), packet.length());
		const icmp_hdr_t hdr =
		{
			.type = deser.ReadU8(),
			.code = deser.ReadU8(),
			.checksum = deser.ReadU16(),
		};

		//An echo reply goes whole to the ICMP socket whose port is its echo id
		if (hdr.type == icmp_type_echo_reply)
		{
			if (packet.length() < icmp_hdr_size + echo_hdr_size)
				return false;

			packet.proto = proto_t::Icmp;
			packet.dst.port = deser.ReadU16();
			return Socket::Receive(net_if, packet);
		}

		packet.mask(icmp_hdr_size);
		if (hdr.type == icmp_type_dest_unreachable)
		{
			ReceiveDestUnreachable(net_if, packet);
			return false;
		}

		//A broadcast echo request gets no reply, which would go out with the broadcast address as its source
		if (hdr.type == icmp_type_echo_request && !net_if.ipv4.is_broadcast(packet.dst.addr))
			ReceiveEchoRequest(net_if, packet);

		return false;
	}

	bool Send(NetIf& net_if, Packet& packet)
	{
		if (packet.headroom() < icmp_hdr_size)
			return false;
		packet.unmask(icmp_hdr_size);

		const icmp_hdr_t hdr =
		{
			.type = get_icmp_type(packet.proto),
			.code = 0,
			.checksum = 0
		};

		Serializer ser(packet.buffer(), packet.length());
		ser.Write(hdr.type);
		ser.Write(hdr.code);
		ser.Write(hdr.checksum);

		const uint16_t expected_checksum = Net::checksum(packet.buffer(), packet.length());
		ser.Reset();
		ser.Skip(icmp_hdr_checksum_offset);
		ser.Write(expected_checksum);

		packet.proto = proto_t::Icmp;
		return IPv4::Send(net_if, packet);
	}
}
