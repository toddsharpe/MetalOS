#include "Net/Net.h"
#include "Net/NetIf.h"
#include "Lib/StaticVector.h"
#include "Assert.h"

namespace
{
	struct route_t
	{
		Net::NetIf* dst_if;
		Net::ipv4_addr_t addr;
		Net::ipv4_addr_t subnet_mask;
	};

	StaticVector<route_t, 8> m_routes;

	static bool ipv4_valid(Net::NetIf& net_if)
	{
		return net_if.ipv4.addr != Net::empty;
	}
}

namespace Net
{
	extern StaticVector<Net::NetIf*, 8> m_netIfs;
}

namespace Net::Router
{
	NetIf& Resolve(const Net::endpoint_t& dst)
	{
		//Route based on manual routes
		for (const route_t& route : m_routes)
		{
			if (!ipv4_valid(*route.dst_if))
				continue;
			
			const bool is_network = Net::is_network(dst.addr, route.addr, route.subnet_mask);
			if (!is_network)
				continue;

			return *route.dst_if;
		}

		//Route based on interface subnet/multicast
		for (NetIf* net_if : m_netIfs)
		{
			if (!ipv4_valid(*net_if))
				continue;

			const bool is_network = Net::is_network(dst.addr, net_if->ipv4.addr, net_if->ipv4.subnet_mask);
			const bool is_multicast = Net::is_multicast(dst.addr);
			const bool is_joined = net_if->ipv4.is_joined(dst.addr);
			const bool accept = is_multicast ? is_joined : is_network;
			if (!accept)
				continue;

			return *net_if;
		}

		return *m_netIfs.First();
	}

	bool AddRoute(NetIf& net_if, const ipv4_addr_t addr, const ipv4_addr_t subnet_mask)
	{
		Assert(m_netIfs.Contains(&net_if));
		return m_routes.Add({ &net_if, addr, subnet_mask });
	}
}
