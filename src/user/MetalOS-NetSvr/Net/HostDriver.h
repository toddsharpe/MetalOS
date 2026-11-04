#pragma once

#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "user/MetalOS.h"

//The netstack's NIC shim: TX via the NetSend syscall, RX drained from the kernel ring by the main loop (so Receive() is a no-op), MAC from GetInterfaces.
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

	milli_t GetTicks() const override
	{
		return GetTickCount();
	}

private:
	InterfaceInfo m_info;
};
