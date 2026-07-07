#pragma once

#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "user/MetalOS.h"

//The netstack's "NIC driver": a thin shim over the kernel net device. Transmit goes
//through the NetSend syscall (the kernel copies the frame to the real NIC); receive is
//driven by the main loop draining the kernel->netstack RX ring, so the pull-style
//Receive() here is a no-op. The MAC comes from the kernel (published under "net.mac").
class HostDriver : public Net::NetDriver
{
public:
	explicit HostDriver(const InterfaceInfo& info) :
		m_info(info)
	{

	}

	void Receive(Net::NetIf&) override
	{

	}

	bool Send(Net::NetIf& netIf, Net::Packet& packet) override
	{
		return NetSend(m_info.index, packet.buffer(), packet.length()) == SyscallResult::Success;
	}

	const Net::eth_mac_t& GetMac() const override
	{
		static Net::eth_mac_t mac = {};
		memcpy(mac.bytes, m_info.mac, sizeof(mac.bytes));
		return mac;
	}

	bool IsLinkUp() const override
	{
		return true;
	}

private:
	InterfaceInfo m_info;
};
