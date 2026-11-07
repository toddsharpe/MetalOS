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

	//The candidate with the longest subnet mask, the most specific
	struct pick_t
	{
		Net::NetIf* net_if;
		Net::ipv4_addr_t mask;

		void consider(Net::NetIf* const candidate, const Net::ipv4_addr_t candidate_mask)
		{
			if (net_if != nullptr && candidate_mask.addr <= mask.addr)
				return;

			net_if = candidate;
			mask = candidate_mask;
		}
	};

	//Where an address no interface claims goes: the first interface that is up, else the first, else nowhere
	Net::NetIf* Default()
	{
		const Span<Net::NetIf* const> net_ifs = Net::GetNetIfs();
		for (Net::NetIf* const net_if : net_ifs)
		{
			if (net_if->IsUp())
				return net_if;
		}
		return net_ifs.Size == 0 ? nullptr : net_ifs[0];
	}
}

//Ported from Nighthawk's, under the manual routes this stack keeps for more interfaces
namespace Net::Router
{
	NetIf* Resolve(const ipv4_addr_t addr)
	{
		//Route based on manual routes
		for (const route_t& route : m_routes)
		{
			if (!ipv4_valid(*route.dst_if))
				continue;

			if (Net::is_network(addr, route.addr, route.subnet_mask))
				return route.dst_if;
		}

		//Route based on interface subnet or group: an up interface wherever there is a choice, a down one only when no up one matches
		const bool is_multicast = Net::is_multicast(addr);
		pick_t up = {};
		pick_t down = {};
		for (NetIf* const net_if : GetNetIfs())
		{
			if (!is_multicast && !ipv4_valid(*net_if))
				continue;

			const bool accept = is_multicast ? net_if->ipv4.is_joined(addr) : Net::is_network(addr, net_if->ipv4.addr, net_if->ipv4.subnet_mask);
			if (!accept)
				continue;

			(net_if->IsUp() ? up : down).consider(net_if, is_multicast ? Net::empty : net_if->ipv4.subnet_mask);
		}
		if (up.net_if != nullptr)
			return up.net_if;
		if (down.net_if != nullptr)
			return down.net_if;

		return Default();
	}

	bool AddRoute(NetIf& net_if, const ipv4_addr_t addr, const ipv4_addr_t subnet_mask)
	{
		Assert(HasNetIf(net_if));
		return m_routes.Add({ &net_if, addr, subnet_mask });
	}
}
