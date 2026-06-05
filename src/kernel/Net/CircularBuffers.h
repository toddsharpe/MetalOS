#pragma once

#include "Core/Buffer.h"
#include <cstddef>
#include <array>

namespace Net
{
	template <size_t T_N, size_t T_Size>
	class CircularBuffers
	{
	public:
		CircularBuffers() : m_buffers(),
							m_read(),
							m_write()
		{
			static_assert(T_N > 0 && (T_N & (T_N - 1)) == 0);
		}

		Buffer &Write()
		{
			static Buffer empty = {};
			if (IsFull())
				return empty;

			const size_t write_idx = mask(m_write++);
			return m_buffers[write_idx];
		}

		const Buffer &Read()
		{
			static Buffer empty = {};
			if (IsEmpty())
				return empty;

			const size_t read_idx = mask(m_read++);
			return m_buffers[read_idx];
		}

		size_t Size() const
		{
			return m_write - m_read;
		}

		bool IsEmpty() const
		{
			return m_read == m_write;
		}

		bool IsFull() const
		{
			return Size() == T_N;
		}

	private:
		constexpr size_t mask(const uint32_t value)
		{
			constexpr size_t bitmask = T_N - 1;
			return value & bitmask;
		}

		using entry_t = StaticBuffer<T_Size>;

		std::array<entry_t, T_N> m_buffers;
		uint32_t m_read;
		uint32_t m_write;
	};
}
