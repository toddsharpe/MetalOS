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

		//Unresolved next hop: ARP for it, and the packet waits here for the reply
		eth_mac_t dst_mac = {};
		if (!ArpCache::Lookup(nexthop, dst_mac))
		{
			eth_data_t& eth = net_if.ethernet;

			//One ARP at a time; while one is out a sender is refused, so a socket keeps its datagram queued
			if (!eth.arp.try_begin(net_if.driver.GetTicks()))
				return false;

			//A frame still waiting on an expired request is dropped for this one
			if (eth.pending.backing)
			{
				free_tx(eth.pending.backing);
				eth.pending = {};
			}

			//Build arp (reserve room for the ethernet + ARP headers)
			const buffer_t buffer = alloc_tx(buffer_size);
			if (!buffer)
			{
				eth.arp.end();
				return false;
			}

			Packet arp(buffer);
			arp.mask(eth_hdr_size);
			arp.mask(arp_hdr_size);
			arp.dst.addr = nexthop;

			//Issue arp
			if (!Arp::Send(net_if, arp))
			{
				free_tx(buffer);
				eth.arp.end();
				return false;
			}

			//The packet is taken: it goes when Dispatch sees the reply, or is dropped when the request expires
			eth.arp.target = nexthop;
			eth.pending = packet;
			return true;
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

	void Dispatch(NetIf& net_if)
	{
		eth_data_t& eth = net_if.ethernet;
		if (!eth.pending.backing)
			return;

		eth_mac_t mac = {};
		const bool resolved = ArpCache::Lookup(eth.arp.target, mac);
		if (!resolved && !eth.arp.expired(net_if.driver.GetTicks()))
			return;

		//One attempt: a frame that can't go now is dropped, as a lost datagram is
		Packet frame = eth.pending;
		eth.pending = {};
		if (!resolved || !Send(net_if, frame))
			free_tx(frame.backing);
	}
}
