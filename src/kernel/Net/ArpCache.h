#pragma once

#include "Net/Net.h"

namespace Net::ArpCache
{
	bool Contains(const ipv4_addr_t& ip_addr);
	bool Update(const ipv4_addr_t& ip_addr, const eth_mac_t& eth_addr);
	bool Lookup(const ipv4_addr_t& ipAddr, eth_mac_t& eth_addr);

	void Display();
}
