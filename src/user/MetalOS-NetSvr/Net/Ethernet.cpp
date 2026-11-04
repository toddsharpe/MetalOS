#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"

namespace Net::Ethernet
{
	static bool Accept(NetIf& net_if, const eth_mac_t& dst)
	{
		return eth_broadcast == dst || net_if.ethernet.addr == dst || net_if.ethernet.filters.Contains(dst);
	}

	bool Receive(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < eth_hdr_size)
			return false;
		
		Deserializer deser(packet.buffer(), packet.length());
		eth_mac_t dst = {};
		deser.Read(dst.bytes);
		eth_mac_t src = {};
		deser.Read(src.bytes);

		const eth_hdr_t hdr =
		{
			.dst = dst,
			.src = src,
			.type = deser.ReadU16(),
		};
		packet.mask(eth_hdr_size);

		if (Accept(net_if, hdr.dst))
		{
			switch (hdr.type)
			{
				case eth_type_ipv4:
					return IPv4::Receive(net_if, packet);

				case eth_type_arp:
					return Arp::Receive(net_if, packet);
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	bool Send(NetIf& net_if, Packet& packet)
	{
		if (packet.headroom() < eth_hdr_size)
			return false;

		//Broadcast IP, limited or this subnet's, maps directly to the broadcast MAC (no ARP, e.g. DHCP DISCOVER).
		const bool is_broadcast = net_if.ipv4.is_broadcast(packet.dst.addr);

		//L2 next hop: on-subnet destinations are reached directly; anything else goes via
		//the default gateway (which must itself be on-subnet).
		ipv4_addr_t nexthop = is_broadcast ? Net::broadcast : packet.dst.addr;
		if (!is_broadcast && !Net::is_network(packet.dst.addr, net_if.ipv4.addr, net_if.ipv4.subnet_mask))
			nexthop = net_if.ipv4.gateway;

		//Off-subnet with no gateway set: there is no next hop to ARP for
		if (nexthop == Net::empty)
			return false;

		//Unresolved next hop: drop the packet and ARP for it (senders retry once the cache is warm)
		eth_mac_t dst_mac = {};
		if (!ArpCache::Lookup(nexthop, dst_mac))
		{
			//One ARP at a time; an unanswered one expires so a later send asks again
			if (!net_if.ethernet.arp.try_begin(net_if.driver.GetTicks()))
				return false;

			//Build arp (reserve room for the ethernet + ARP headers)
			StaticBuffer<Net::buffer_size> buffer;
			Packet arp(buffer);
			arp.mask(eth_hdr_size);
			arp.mask(arp_hdr_size);
			arp.dst.addr = nexthop;

			//Issue arp
			return Arp::Send(net_if, arp);
		}

		const eth_hdr_t hdr =
		{
			.dst = dst_mac,
			.src = net_if.ethernet.addr,
			.type = get_eth_type(packet.proto)
		};
		packet.unmask(eth_hdr_size);

		Serializer ser(packet.buffer(), packet.length());
		ser.Write(hdr.dst.bytes);
		ser.Write(hdr.src.bytes);
		ser.Write(hdr.type);

		return net_if.driver.Send(net_if, packet);
	}
}
