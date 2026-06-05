#include "Net/NetIf.h"
#include "Net/Packet.h"
#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"

namespace Net::IPv4
{
	static void Forward(const Packet& packet, NetIf& dst_if)
	{
#if 0
		printf("Fwd %d.%d.%d.%d -> %p\n",
			packet.dst.addr.bytes[3], packet.dst.addr.bytes[2], packet.dst.addr.bytes[1], packet.dst.addr.bytes[0],
			&dst_if
		);
#endif
		//Get buffer
		const NetLayer& l2 = get_l2(dst_if.l2);
		StaticBuffer<Net::buffer_size> buffer;
		Copy(buffer, packet.backing.Ref());

		//Clone packet
		Net::Packet cloned(buffer);
		cloned.src = packet.src;
		cloned.dst = packet.dst;
		cloned.proto = packet.proto;
		cloned.mask(l2.HeaderSize);

		//Copy data
		memcpy(cloned.buffer(), packet.buffer(), packet.length());
		cloned.trim(packet.length());

		//Pop ip hdr and send
		l2.Send(dst_if, cloned);
	}

	bool Meta(Packet& packet)
	{
		if (packet.length() < ipv4_hdr_size)
		{
			return false;
		}

		Deserializer deser(packet.buffer(), packet.length());
		const ipv4_hdr_t ip_hdr =
		{
			.ihl_version = deser.ReadU8(),
			.dscp = deser.ReadU8(),
			.length = deser.ReadU16(),
			.id = deser.ReadU16(),
			.fragment_flags = deser.ReadU16(),
			.ttl = deser.ReadU8(),
			.proto = deser.ReadU8(),
			.checksum = deser.ReadU16(),
			.src = {.addr = deser.ReadU32() },
			.dst = {.addr = deser.ReadU32() },
		};
		packet.mask(ipv4_hdr_size);

		//TODO(tsharpe): Checksum

		//Populate packet values
		packet.src = { ip_hdr.src, 0 };
		packet.dst = { ip_hdr.dst, 0 };
		packet.proto = proto_t::Ipv4;

		switch (ip_hdr.proto)
		{
			case ip_proto_udp:
				return Udp::Meta(packet);

			default:
				Assert(false);
				return false;
		}
	}
	
	bool Receive(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < ipv4_hdr_size)
		{
			net_if.ipv4.rx_dropped++;
			return false;
		}

		Deserializer deser(packet.buffer(), packet.length());
		const ipv4_hdr_t ip_hdr =
		{
			.ihl_version = deser.ReadU8(),
			.dscp = deser.ReadU8(),
			.length = deser.ReadU16(),
			.id = deser.ReadU16(),
			.fragment_flags = deser.ReadU16(),
			.ttl = deser.ReadU8(),
			.proto = deser.ReadU8(),
			.checksum = deser.ReadU16(),
			.src = {.addr = deser.ReadU32() },
			.dst = {.addr = deser.ReadU32() },
		};

		//TODO(tsharpe): Checksum

		//Populate packet values
		packet.src = { ip_hdr.src, 0 };
		packet.dst = { ip_hdr.dst, 0 };
		packet.proto = proto_t::Ipv4;

#if 1
		printf("IPv4: %d.%d.%d.%d -> %d.%d.%d.%d\n",
			packet.src.addr.bytes[3], packet.src.addr.bytes[2], packet.src.addr.bytes[1], packet.src.addr.bytes[0],
			packet.dst.addr.bytes[3], packet.dst.addr.bytes[2], packet.dst.addr.bytes[1], packet.dst.addr.bytes[0]
		);
#endif

		//Forward packet if needed
		const bool is_broadcast = packet.dst.addr == Net::broadcast;
		const bool is_match = net_if.ipv4.addr == packet.dst.addr;
		const bool is_joined = net_if.ipv4.is_joined(packet.dst.addr);
		const bool is_multicast = Net::is_multicast(packet.dst.addr);
		const bool accept = is_multicast ? is_joined : (is_match || is_broadcast);

		//Update stats
		if (is_broadcast)
			net_if.ipv4.rx_broadcast++;
		else if (is_multicast)
			net_if.ipv4.rx_multicast++;
		else
			net_if.ipv4.rx_unicast++;

		if (is_multicast)
		{
			for (const ipv4_mc_route_t& route : net_if.ipv4.mc_routes)
			{
				if (route.group != packet.dst.addr)
					continue;

				Assert(route.dst);
				Forward(packet, *route.dst);
			}
		}
		else if (!is_match && net_if.ipv4.ip_forwarding)
		{
			//Attempt to forward
			NetIf& target_if = Router::Resolve(packet.dst);
			if (&target_if != &net_if)
				Forward(packet, target_if);
		}
		
		if (accept)
		{
			packet.mask(ipv4_hdr_size);

			net_if.ipv4.rx_accepted++;
			switch (ip_hdr.proto)
			{
				case ip_proto_icmp:
					return Icmp::Receive(net_if, packet);
					break;

				case ip_proto_udp:
					return Udp::Receive(net_if, packet);
					break;

				default:
					printf("Unsupported IPv4 proto: %d\n", ip_hdr.proto);
					return false;
			}
		}
		else
		{
			//Drop
			return false;
		}
	}

	bool Send(NetIf& net_if, Packet& packet)
	{
		if (packet.headroom() < ipv4_hdr_size)
			return false;
		packet.unmask(ipv4_hdr_size);

		const ipv4_hdr_t hdr =
		{
			.ihl_version = 0x45,
			.dscp = 0,
			.length = static_cast<uint16_t>(packet.length()),
			.id = 0,
			.fragment_flags = 0,
			.ttl = 64,
			.proto = get_ipv4_proto(packet.proto),
			.checksum = 0x0000,
			.src = packet.src.addr,
			.dst = packet.dst.addr,
		};

		Serializer ser(packet.buffer(), packet.length());
		ser.Write(hdr.ihl_version);
		ser.Write(hdr.dscp);
		ser.Write(hdr.length);
		ser.Write(hdr.id);
		ser.Write(hdr.fragment_flags);
		ser.Write(hdr.ttl);
		ser.Write(hdr.proto);
		ser.Write(hdr.checksum);
		ser.Write(hdr.src.addr);
		ser.Write(hdr.dst.addr);

		const uint16_t checksum = Net::checksum(packet.buffer(), sizeof(hdr));
		ser.Reset();
		ser.Skip(ipv4_hdr_checksum_offset);
		ser.Write(checksum);

		packet.proto = proto_t::Ipv4;

		const NetLayer& l2 = get_l2(net_if.l2);
		return l2.Send(net_if, packet);
	}
}
