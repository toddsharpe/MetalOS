#pragma once

#include "Lib/RingBuffer.h"
#include "Lib/Buffer.h"
#include "Assert.h"
#include "core_crt/stdint.h"

namespace Net
{
	// Variable-length packet queue backed by a byte ring buffer.
	// Each packet is stored as a uint32_t length prefix followed by its data bytes.
	// N must be a power of two (enforced by RingBuffer).
	template <size_t N>
	class PacketBuffer
	{
	public:
		constexpr PacketBuffer() :
			m_ring(),
			m_count()
		{
		}

		bool Push(const void *const data, const size_t length)
		{
			const uint32_t len = static_cast<uint32_t>(length);
			if (m_ring.Size() + sizeof(len) + length > N)
				return false;

			m_ring.Push(&len, sizeof(len));
			m_ring.Push(data, length);
			m_count++;
			return true;
		}

		bool Push(const CBuffer &packet)
		{
			return Push(packet.Data, packet.Size);
		}

		bool Pop(Buffer &out, size_t &bytes_read)
		{
			if (IsEmpty())
				return false;

			uint32_t len = 0;
			size_t dummy = 0;
			m_ring.Pop(&len, sizeof(len), dummy);
			Assert(dummy == sizeof(len));

			Assert(len <= out.Capacity);
			m_ring.Pop(out.Data, len, bytes_read);
			m_count--;
			return true;
		}

		bool IsEmpty() const
		{
			return m_count == 0;
		}

		size_t Count() const
		{
			return m_count;
		}

	private:
		RingBuffer<N> m_ring;
		size_t m_count;
	};
}
