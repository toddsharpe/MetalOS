#include "Net/Net.h"
#include "Lib/CircularBuffer.h"

namespace
{
	struct entry_t
	{
		Net::ipv4_addr_t ip_addr;
		Net::eth_mac_t eth_addr;
	};
	constexpr size_t CacheSize = 8;

	//Broadcast and multicast MACs are computed by Lookup, so every entry is a unicast peer, oldest first
	CircularBuffer<entry_t, CacheSize> cache;
}

namespace Net::ArpCache
{
	void Update(const Net::ipv4_addr_t& ip_addr, const Net::eth_mac_t& eth_addr)
	{
		//0.0.0.0 is the sender of an ARP probe, not a peer, so it's never cached
		if (ip_addr == Net::empty)
			return;

		//Search for existing device
		for (size_t i = 0; i < cache.Count(); i++)
		{
			if (cache[i].ip_addr != ip_addr)
				continue;

			cache[i].eth_addr = eth_addr;
			return;
		}

		//Full: the oldest peer makes room for the new one
		if (cache.IsFull())
			cache.Dequeue();
		Assert(cache.Enqueue({ ip_addr, eth_addr }));
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
		
		for (size_t i = 0; i < cache.Count(); i++)
		{
			if (cache[i].ip_addr == ip_addr)
			{
				eth_addr = cache[i].eth_addr;
				return true;
			}
		}

		return false;
	}
}
