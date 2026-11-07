#include "Net/Net.h"
#include "Lib/StaticVector.h"
#include "Lib/BlockAllocator.h"

namespace Net
{
	static StaticVector<Net::NetIf*, 8> m_netIfs;

	stack_stats_t stack_stats = {};

	static uint8_t m_rxStorage[pool_size];
	static constinit BlockAllocator<pool_size> m_rxPool(m_rxStorage);
	static uint8_t m_txStorage[pool_size];
	static constinit BlockAllocator<pool_size> m_txPool(m_txStorage);

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

	buffer_t alloc_rx(const size_t bytes)
	{
		const buffer_t buffer = m_rxPool.Allocate(bytes);
		if (!buffer)
			stack_stats.pool_rx_empty++;
		return buffer;
	}

	void free_rx(const buffer_t buffer)
	{
		m_rxPool.Free(buffer);
	}

	buffer_t alloc_tx(const size_t bytes)
	{
		const buffer_t buffer = m_txPool.Allocate(bytes);
		if (!buffer)
			stack_stats.pool_tx_empty++;
		return buffer;
	}

	void free_tx(const buffer_t buffer)
	{
		m_txPool.Free(buffer);
	}
}
