#include "Net/Net.h"
#include "Lib/StaticVector.h"

namespace Net
{
	StaticVector<Net::NetIf*, 8> m_netIfs;

	bool AddNetIf(NetIf& net_if)
	{
		return m_netIfs.Add(&net_if);
	}
}
