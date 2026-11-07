#pragma once

#include "Net/Net.h"
#include "Net/Packet.h"
#include "Net/NetDriver.h"
#include "Lib/StaticVector.h"

namespace Net
{
	struct eth_data_t
	{
		constexpr eth_data_t() :
			addr(),
			filters(),
			arp()
		{

		}

		eth_mac_t addr;
		StaticVector<eth_mac_t, 4> filters; //extra accepted destination MACs (joined multicast groups)
		arp_request_t arp;
	};

	struct ipv4_mc_route_t
	{
		ipv4_addr_t group;
		NetIf *dst;
	};
	
	struct ipv4_data_t
	{
		constexpr ipv4_data_t() : mc_groups(), mc_routes(), addr(), subnet_mask(Net::subnet_32), gateway(), ip_forwarding(false), rx_dropped(), rx_accepted(), rx_unicast(), rx_multicast(), rx_broadcast()
		{
		}

		bool is_joined(const ipv4_addr_t mc_group) const
		{
			return mc_groups.Contains(mc_group);
		}

		//The limited broadcast, or this interface's directed broadcast
		bool is_broadcast(const ipv4_addr_t address) const
		{
			return address == Net::broadcast || Net::is_subnet_broadcast(address, addr, subnet_mask);
		}

		/*
		 * Config.
		 */
		StaticVector<ipv4_addr_t, 4> mc_groups;
		StaticVector<ipv4_mc_route_t, 4> mc_routes;
		ipv4_addr_t addr;
		ipv4_addr_t subnet_mask;
		ipv4_addr_t gateway;
		bool ip_forwarding; //Off unless set: a host must not route others' packets by default (RFC 1122)

		/*
		 * Stats.
		 */
		uint32_t rx_dropped;
		uint32_t rx_accepted;
		uint32_t rx_unicast;
		uint32_t rx_multicast;
		uint32_t rx_broadcast;
	};
	
	class NetDriver;
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

		NetDriver& driver;
		const l2_t l2;

		ipv4_data_t ipv4;
		eth_data_t ethernet;
	};

	const NetLayer& get_l2(const l2_t l2);
}
