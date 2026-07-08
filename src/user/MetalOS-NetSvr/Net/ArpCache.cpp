#include "Net/Net.h"
#include "Lib/StaticArray.h"

namespace
{
	struct entry_t
	{
		Net::ipv4_addr_t ip_addr;
		Net::eth_mac_t eth_addr;
	};
	StaticArray<entry_t, 4> cache = {{Net::broadcast, Net::eth_broadcast}};
}

namespace Net::ArpCache
{
	bool Contains(const Net::ipv4_addr_t& ip_addr)
	{
		//Multicast or broadcast doesnt need arp
		if (Net::is_multicast(ip_addr) || ip_addr == Net::broadcast)
			return true;
		
		for (const entry_t& entry : cache)
		{
			if (entry.ip_addr == ip_addr)
				return true;
		}

		return false;
	}

	bool Update(const Net::ipv4_addr_t& ip_addr, const Net::eth_mac_t& eth_addr)
	{
		//Search for existing device
		for (entry_t& entry : cache)
		{
			if (entry.ip_addr != ip_addr)
				continue;

			entry.eth_addr = eth_addr;
			return true;
		}
		
		//Add into empty slot
		for (entry_t& entry : cache)
		{
			if (entry.ip_addr != Net::empty)
				continue;

			entry.ip_addr = ip_addr;
			entry.eth_addr = eth_addr;
			return true;
		}

		return false;
	}

	bool Lookup(const Net::ipv4_addr_t& ip_addr, Net::eth_mac_t& eth_addr)
	{
		if (ip_addr == Net::broadcast)
		{
			eth_addr = Net::eth_broadcast;
			return true;
		}
		else if (Net::is_multicast(ip_addr))
		{
			eth_addr =
			{
				0x01,
				0x00,
				0x5E,
				(uint8_t)(0x7F & ip_addr.bytes[2]),
				ip_addr.bytes[1],
				ip_addr.bytes[0],
			};
			return true;
		}
		
		for (const entry_t& entry : cache)
		{
			if (entry.ip_addr == ip_addr)
			{
				eth_addr = entry.eth_addr;
				return true;
			}
		}

		return false;
	}

	void Display()
	{
		printf("ArpCache:\n");
		for (const entry_t& entry : cache)
		{
			if (entry.ip_addr == Net::empty)
			{
				printf("    <free>\n");
			}
			else
			{
				printf("    %d.%d.%d.%d -> %02X:%02X:%02X:%02X:%02X:%02X\n",
					entry.ip_addr.bytes[3], entry.ip_addr.bytes[2], entry.ip_addr.bytes[1], entry.ip_addr.bytes[0],
					entry.eth_addr.bytes[5], entry.eth_addr.bytes[4], entry.eth_addr.bytes[3], entry.eth_addr.bytes[2], entry.eth_addr.bytes[1], entry.eth_addr.bytes[0]);
			}
		}
	}
}
