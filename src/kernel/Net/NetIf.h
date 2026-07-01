#pragma once

#include "Net/Net.h"
#include "Net/Packet.h"
#include "Net/BufferPool.h"
#include "Net/NetDriver.h"
#include "Lib/StaticArray.h"

namespace Net
{
	struct eth_data_t
	{
		constexpr eth_data_t() :
			addr(),
			filters(),
			pending_tx(),
			arp_pending()
		{

		}
		
		eth_mac_t addr;
		StaticArray<eth_mac_t, 4> filters;
		Packet pending_tx;
		bool arp_pending;
	};

	struct ipv4_mc_route_t
	{
		ipv4_addr_t group;
		NetIf *dst;
	};
	
	struct ipv4_data_t
	{
		constexpr ipv4_data_t() : mc_groups(), mc_routes(), addr(), subnet_mask(Net::subnet_32), gateway(), ip_forwarding(true), rx_dropped(), rx_accepted(), rx_unicast(), rx_multicast(), rx_broadcast()
		{
		}

		bool is_joined(const ipv4_addr_t mc_group) const
		{
			for (const ipv4_addr_t &addr : mc_groups)
			{
				if (addr == mc_group)
					return true;
			}
			return false;
		}

		/*
		 * Config.
		 */
		StaticArray<ipv4_addr_t, 4> mc_groups;
		StaticArray<ipv4_mc_route_t, 4> mc_routes;
		ipv4_addr_t addr;
		ipv4_addr_t subnet_mask;
		ipv4_addr_t gateway;
		bool ip_forwarding;

		/*
		 * Stats.
		 */
		uint32_t rx_dropped;
		uint32_t rx_accepted;
		uint32_t rx_unicast;
		uint32_t rx_multicast;
		uint32_t rx_broadcast;
	};
	
	struct NetDriver;
	struct NetIf
	{
		NetIf(NetDriver& _driver, const l2_t _l2)
			: driver(_driver)
			, l2(_l2)
			, ipv4()
			, ethernet()
		{

		}

		void Init()
		{
			ethernet.addr = driver.GetMac();
		}

		void PreDispatch()
		{
			driver.Receive(*this);
		}

		void PostDispatch()
		{
			if (l2 == l2_t::Ethernet)
				Net::Ethernet::TxFlush(*this);
		}

		NetDriver& driver;
		const l2_t l2;

		ipv4_data_t ipv4;
		eth_data_t ethernet;
	};

	const NetLayer EthernetLayer = { Ethernet::Receive, Ethernet::Send, eth_hdr_size };
	constexpr const NetLayer& get_l2(const l2_t l2)
	{
		switch (l2)
		{
			case l2_t::Ethernet:
				return EthernetLayer;

#if defined (LINUX)
			case l2_t::Tun:
			{
				static const NetLayer TunEthLayer = { TunEth::Receive, TunEth::Send, tun_hdr_size };
				return TunEthLayer;
			}
#endif

			default:
				Assert(false);
				return EthernetLayer;
		}
	}
}
