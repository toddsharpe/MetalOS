#pragma once

#include "Net/Net.h"
#include "Lib/Buffer.h"
#include "Lib/StaticArray.h"
#include "Lib/Bitset.h"
#include "core_crt/stdint.h"
#include "core_crt/stddef.h"

namespace Net
{
	template <size_t N, size_t Length = Net::buffer_size>
	class BufferPool
	{
	public:
		constexpr BufferPool() :
			m_entries(),
			m_inUse()
		{

		}

		const Buffer Acquire()
		{
			for (size_t i = 0; i < m_entries.size(); i++)
			{
				if (m_inUse[i])
					continue;

				m_inUse[i] = true;
				return m_entries[i];
			}

			return {};
		}

		bool Release(const Buffer& buffer)
		{
			for (size_t i = 0; i < m_entries.size(); i++)
			{
				if (m_entries[i].Data != buffer.Data)
					continue;

				//Assert(m_inUse[i]);
				m_inUse[i] = false;
				return true;
			}

			Assert(false);
			return false;
		}

		void Display() const
		{
			printf("BufferPool::Display\n");
			for (size_t i = 0; i < m_entries.size(); i++)
			{
				printf("entries[%d] = { 0x%08x, %d }\n", i, m_entries[i].Data, m_inUse[i]);
			}
		}

	private:
		using entry_t = StaticBuffer<Net::buffer_size>;
		StaticArray<entry_t, N> m_entries;
		StaticBitset<N> m_inUse;
	};
}
