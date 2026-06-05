#include "Net/Net.h"
#include "Net/NetIf.h"
#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"

namespace Net::Arp
{
	static constexpr uint16_t htype_ethernet = 1;
	static constexpr uint16_t hw_len = 6;
	static_assert(hw_len == sizeof(eth_mac_t));
	static constexpr uint16_t proto_len = 4;
	static_assert(proto_len == sizeof(ipv4_addr_t));
	
	static void RequestReceive(NetIf& net_if, const arp_t& request)
	{
		//Check query target
		if (request.target_proto != net_if.ipv4.addr)
			return;

		//Add sender to arp cache
		Assert(ArpCache::Update(request.sender_proto, request.sender_hw));

		const arp_t reply =
		{
			.hw_type = request.hw_type,
			.proto = request.proto,
			.hw_len = request.hw_len,
			.proto_len = request.proto_len,
			.op = arp_op_reply,
			.sender_hw = net_if.ethernet.addr,
			.sender_proto = net_if.ipv4.addr,
			.target_hw = request.sender_hw,
			.target_proto = request.sender_proto,
		};

		const NetLayer& l2 = get_l2(net_if.l2);

		StaticBuffer<Net::buffer_size> buffer;
		Packet packet(buffer);
		packet.mask(l2.HeaderSize);

		//Write packet out
		Serializer ser(packet.buffer(), packet.length());
		ser.Write(reply.hw_type);
		ser.Write(reply.proto);
		ser.Write(reply.hw_len);
		ser.Write(reply.proto_len);
		ser.Write(reply.op);
		ser.Write(reply.sender_hw.bytes);
		ser.Write(reply.sender_proto.bytes);
		ser.Write(reply.target_hw.bytes);
		ser.Write(reply.target_proto.bytes);
		packet.trim(ser.Bytes());

		//Send reply
		packet.proto = proto_t::Arp;
		packet.dst.addr = Net::broadcast;
		l2.Send(net_if, packet);
	}

	static void ReplyReceive(NetIf& net_if, const arp_t& reply)
	{
		//Check reply target
		if (reply.target_proto != net_if.ipv4.addr)
			return;

		Assert(ArpCache::Update(reply.sender_proto, reply.sender_hw));
	}
	
	bool Receive(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < arp_hdr_size)
			return false;

		Deserializer deser(packet.buffer(), packet.length());
		arp_t hdr =
		{
			.hw_type = deser.ReadU16(),
			.proto = deser.ReadU16(),
			.hw_len = deser.ReadU8(),
			.proto_len = deser.ReadU8(),
			.op = deser.ReadU16(),
		};

		uint8_t sender_proto[proto_len] = {};
		uint8_t target_proto[proto_len] = {};

		deser.Read(hdr.sender_hw.bytes);
		deser.Read(sender_proto);
		deser.Read(hdr.target_hw.bytes);
		deser.Read(target_proto);

		memcpy(hdr.sender_proto.bytes, sender_proto, sizeof(sender_proto));
		memcpy(hdr.target_proto.bytes, target_proto, sizeof(target_proto));

		switch (hdr.op)
		{
			case arp_op_request:
				RequestReceive(net_if, hdr);
				break;

			case arp_op_reply:
				ReplyReceive(net_if, hdr);
				break;
		}

		return false;
	}

	bool Send(NetIf& net_if, Packet& packet)
	{
		if (packet.headroom() < arp_hdr_size)
			return false;
		packet.unmask(arp_hdr_size);

		//Build arp request
		const arp_t arp =
		{
			.hw_type = htype_ethernet,
			.proto = eth_type_ipv4,
			.hw_len = hw_len,
			.proto_len = proto_len,
			.op = arp_op_request,
			.sender_hw = net_if.ethernet.addr,
			.sender_proto = net_if.ipv4.addr,
			.target_hw = {},
			.target_proto = packet.dst.addr
		};

		//Write packet out
		Serializer ser(packet.buffer(), packet.length());
		ser.Write(arp.hw_type);
		ser.Write(arp.proto);
		ser.Write(arp.hw_len);
		ser.Write(arp.proto_len);
		ser.Write(arp.op);
		ser.Write(arp.sender_hw.bytes);
		ser.Write(arp.sender_proto.bytes);
		ser.Write(arp.target_hw.bytes);
		ser.Write(arp.target_proto.bytes);
		packet.trim(ser.Bytes());

		packet.proto = proto_t::Arp;
		packet.dst.addr = Net::broadcast;

		const NetLayer& l2 = get_l2(net_if.l2);
		return l2.Send(net_if, packet);
	}
}
