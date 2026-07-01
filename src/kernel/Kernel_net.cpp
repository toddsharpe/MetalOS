#include "kernel/Api.h"
#include "kernel/Net/NetIf.h"
#include "Lib/StaticVector.h"
#include "Assert.h"

using namespace Net;

//Interface registry (defined in Net/Net.cpp).
namespace Net
{
	extern StaticVector<NetIf*, 8> m_netIfs;
}

/*
 * Network interface configuration. Interfaces are indexed by position in the registry.
 */
size_t KeNetInterfaceCount()
{
	return Net::m_netIfs.Count();
}

bool KeNetGetInterface(const size_t index, ipv4_addr_t& addr, ipv4_addr_t& subnet, ipv4_addr_t& gateway, eth_mac_t& mac)
{
	if (index >= Net::m_netIfs.Count())
		return false;

	const NetIf* const net_if = Net::m_netIfs[index];
	addr = net_if->ipv4.addr;
	subnet = net_if->ipv4.subnet_mask;
	gateway = net_if->ipv4.gateway;
	mac = net_if->ethernet.addr;
	return true;
}

bool KeNetSetInterface(const size_t index, const ipv4_addr_t& addr, const ipv4_addr_t& subnet)
{
	if (index >= Net::m_netIfs.Count())
		return false;

	NetIf* const net_if = Net::m_netIfs[index];
	net_if->ipv4.addr = addr;
	net_if->ipv4.subnet_mask = subnet;
	return true;
}

bool KeNetSetGateway(const size_t index, const ipv4_addr_t& gateway)
{
	if (index >= Net::m_netIfs.Count())
		return false;

	Net::m_netIfs[index]->ipv4.gateway = gateway;
	return true;
}
