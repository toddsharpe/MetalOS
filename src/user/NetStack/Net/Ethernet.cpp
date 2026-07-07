#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"

namespace Net::Ethernet
{
	static bool Accept(NetIf& net_if, const eth_mac_t& dst)
	{
		if (eth_broadcast == dst)
			return true;
		
		if (net_if.ethernet.addr == dst)
			return true;

		for (const eth_mac_t& filter : net_if.ethernet.filters)
		{
			if (filter == dst)
				return true;
		}

		return false;
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

		//Broadcast IP maps directly to the broadcast MAC (no ARP, e.g. DHCP DISCOVER).
		const bool is_broadcast = packet.dst.addr == Net::broadcast;

		//L2 next hop: on-subnet destinations are reached directly; anything else goes via
		//the default gateway (which must itself be on-subnet).
		ipv4_addr_t nexthop = packet.dst.addr;
		if (!is_broadcast && !Net::is_network(packet.dst.addr, net_if.ipv4.addr, net_if.ipv4.subnet_mask))
			nexthop = net_if.ipv4.gateway;

		//If arp isnt resolved, enqueue
		const bool resolved = is_broadcast || ArpCache::Contains(nexthop);
		if (!resolved)
		{
			//One arp supported at a time
			if (net_if.ethernet.arp_pending)
				return false;

			//Save off tx packet
			net_if.ethernet.arp_pending = true;
			net_if.ethernet.pending_tx = packet;

			//Build arp (reserve room for the ethernet + ARP headers)
			StaticBuffer<Net::buffer_size> buffer;
			Packet arp(buffer);
			arp.mask(eth_hdr_size);
			arp.mask(arp_hdr_size);
			arp.dst.addr = nexthop;

			//Issue arp
			return Arp::Send(net_if, arp);
		}
		else
		{
			//Send packet
			eth_mac_t dst_mac = {};
			if (is_broadcast)
				dst_mac = Net::eth_broadcast;
			else
				Assert(ArpCache::Lookup(nexthop, dst_mac));
	
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

	void TxFlush(NetIf& net_if)
	{
		if (!net_if.ethernet.arp_pending)
			return;

		const bool resolved = ArpCache::Contains(net_if.ethernet.pending_tx.dst.addr);
		if (!resolved)
			return;

		//Attempt to send pending packet once. if it couldnt be sent, free the buffer.
		Send(net_if, net_if.ethernet.pending_tx);
		net_if.ethernet.pending_tx = Packet();
		net_if.ethernet.arp_pending = false;
	}
}
