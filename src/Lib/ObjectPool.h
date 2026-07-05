#pragma once

#include "Lib/Bitset.h"
#include <cstdint>
#include <cstring>
#include "Assert.h"

template <typename T, size_t N>
class ObjectPool
{
public:
	constexpr ObjectPool() : m_storage(), m_used()
	{

	}

	template <typename ...A>
	T* Allocate(A&& ...args)
	{
		size_t index = 0;
		bool first = m_used.First(index);
		if (!first)
			return nullptr;

		Assert(!m_used.Get(index));
		m_used.Set(index);
		void* const allocation = &m_storage[index * sizeof(T)];
		return new(allocation) T(args...);
	}

	void Deallocate(const T* const item)
	{
		const size_t index = item - (T*)&m_storage[0];
		Assert(m_used.Get(index));

		memset(&m_storage[index * sizeof(T)], 0, sizeof(T));
		m_used.Clear(index);
	}

	//Invoke func for each live (allocated) object.
	template <typename Func>
	void ForEach(Func func)
	{
		for (size_t i = 0; i < N; i++)
			if (m_used.Get(i))
				func(reinterpret_cast<T*>(&m_storage[i * sizeof(T)]));
	}

	void Display() const
	{
		Printf("ObjectPool: ");
		for (size_t i = 0; i < N; i++)
		{
			const bool inUse = m_used.Get(i);
			Printf("%c", inUse ? 'X' : ' ');
		}
		Printf("\n");
	}

private:
	static constexpr size_t Size = N * sizeof(T);

	alignas(alignof(T)) uint8_t m_storage[Size];
	StaticBitset<N> m_used;
};
