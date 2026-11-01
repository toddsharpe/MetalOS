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
		StaticBuffer<Net::buffer_size> buffer;
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
		Icmp::Send(net_if, reply);
	}

	bool Receive(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < icmp_hdr_size)
			return false;

		Deserializer deser(packet.buffer(), packet.length());
		const icmp_hdr_t hdr =
		{
			.type = deser.ReadU8(),
			.code = deser.ReadU8(),
			.checksum = deser.ReadU16(),
		};

		//Deliver the complete ICMP message (header + payload) to any raw ICMP socket
		//(e.g. ping) before the kernel strips headers for its own handling.
		packet.proto = proto_t::Icmp;
		Socket::Receive(net_if, packet);

		//Only echo requests need a reply from the stack; any other type is just for raw sockets
		packet.mask(icmp_hdr_size);
		if (hdr.type == icmp_type_echo_request)
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
