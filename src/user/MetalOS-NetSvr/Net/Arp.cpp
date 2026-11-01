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

	//Writes the ARP message at the packet's current offset and broadcasts it
	static bool SendMessage(NetIf& net_if, Packet& packet, const arp_t& arp)
	{
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
		return get_l2(net_if.l2).Send(net_if, packet);
	}

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

		StaticBuffer<Net::buffer_size> buffer;
		Packet packet(buffer);
		packet.mask(get_l2(net_if.l2).HeaderSize);
		SendMessage(net_if, packet, reply);
	}

	static void ReplyReceive(NetIf& net_if, const arp_t& reply)
	{
		//Check reply target
		if (reply.target_proto != net_if.ipv4.addr)
			return;

		Assert(ArpCache::Update(reply.sender_proto, reply.sender_hw));

		//Resolution done; allow future ARP requests (senders retry once the cache is warm)
		net_if.ethernet.arp_pending = false;
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

		deser.Read(hdr.sender_hw.bytes);
		deser.Read(hdr.sender_proto.bytes);
		deser.Read(hdr.target_hw.bytes);
		deser.Read(hdr.target_proto.bytes);

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

		return SendMessage(net_if, packet, arp);
	}
}
