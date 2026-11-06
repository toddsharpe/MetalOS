#include "Net/Net.h"
#include "Lib/StaticVector.h"

namespace Net
{
	static StaticVector<Net::NetIf*, 8> m_netIfs;

	bool AddNetIf(NetIf& net_if)
	{
		return m_netIfs.Add(&net_if);
	}

	Span<NetIf* const> GetNetIfs()
	{
		return { m_netIfs.begin(), m_netIfs.Count() };
	}

	bool HasNetIf(NetIf& net_if)
	{
		return m_netIfs.Contains(&net_if);
	}
}
