#pragma once

#include "Net/Packet.h"

namespace Net
{
	struct NetIf;
	
	class NetDriver
	{
	public:
		/*
		 * Issue IO.
		 */
		virtual void Receive(NetIf& net_if) = 0;
		virtual bool Send(Net::NetIf& net_if, Packet& packet) = 0;

		virtual const eth_mac_t& GetMac() const = 0;
		virtual bool IsLinkUp() const = 0;
	};
}
