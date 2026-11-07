#pragma once

#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "Lib/SharedRing.h"
#include "user/MetalOS.h"
#include "user/Protocol_net.h"

//The netstack's NIC shim: RX drained from the kernel ring into the rx pool, TX via the NetSend syscall, MAC from GetInterfaces.
class HostDriver : public Net::NetDriver
{
public:
	HostDriver(const InterfaceInfo& info, void* const rxRegion) :
		m_info(info),
		m_rx(rxRegion)
	{

	}

	void Receive(Net::NetIf& netIf) override
	{
		NetDevice::Frame frame;
		while (m_rx.Dequeue(frame))
		{
			//A dry pool loses the frame, which alloc_rx counts
			const buffer_t backing = Net::alloc_rx(frame.Length);
			if (!backing)
				continue;

			memcpy(backing.Data, frame.Data, frame.Length);
			Net::Packet packet(backing, frame.Length);
			Net::get_l2(netIf.l2).Receive(netIf, packet);
			if (!packet.taken)
				Net::free_rx(backing);
		}
	}

	bool Send(Net::NetIf& netIf, Net::Packet& packet) override
	{
		UNUSED(netIf);
		if (NetSend(m_info.index, packet.buffer(), packet.length()) != SyscallResult::Success)
			return false;

		//NetSend copied the frame out, so the buffer is done with
		Net::free_tx(packet.backing);
		return true;
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
	SharedRing<NetDevice::Frame> m_rx;
};
