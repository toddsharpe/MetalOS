#pragma once

#include "Net/Packet.h"

namespace Net
{
	struct NetIf;

	struct NetDriverOps
	{
		using send_fn       = bool(*)(void* ctx, NetIf& net_if, Packet& packet);
		using receive_fn    = void(*)(void* ctx, NetIf& net_if);
		using is_link_up_fn = bool(*)(void* ctx);

		void* ctx;
		eth_mac_t mac;
		send_fn send;
		receive_fn receive;
		is_link_up_fn is_link_up;
	};
}
