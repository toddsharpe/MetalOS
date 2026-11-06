#include "Net/NetIf.h"

namespace Net
{
	static const NetLayer EthernetLayer = { Ethernet::Receive, Ethernet::Send, eth_hdr_size };

	const NetLayer& get_l2(const l2_t l2)
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
