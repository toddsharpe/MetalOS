#pragma once

#include "core_crt/stdint.h"
#include "core_crt/string.h"

//Linear map, not an actual HashMap
template <typename TKey, typename TValue, size_t N>
class StaticMap
{
public:
	struct Entry
	{
		TKey Key;
		TValue Value;
	};

	constexpr StaticMap() :
		m_count(),
		m_storage()
	{

	}

	bool Add(const TKey key, TValue value)
	{
		if (m_count == N)
			return false;

		m_storage[m_count] = { key, value };
		m_count++;
		return true;
	}

	bool Remove(const TKey key)
	{
		size_t index = 0;
		if (!Find(key, index))
			return false;

		if (index < m_count - 1)
		{
			//Shift items down
			const size_t num = m_count - index - 1;
			memcpy(&m_storage[index], &m_storage[index + 1], num * sizeof(Entry));
		}

		m_count--;
		return true;
	}

	TValue Get(const TKey key)
	{
		size_t index = 0;
		if (!Find(key, index))
			return (TValue)0;

		return m_storage[index].Value;
	}

	bool Get(const TKey key, TValue& value)
	{
		size_t index = 0;
		if (!Find(key, index))
			return false;

		value = m_storage[index].Value;
		return true;
	}

	size_t Count() const
	{
		return m_count;
	}

	void Display() const
	{
		Printf("StaticMap::Display: Count: %d\n", m_count);
	}

	const Entry *begin() const { return reinterpret_cast<const Entry*>(&m_storage[0]); }
	const Entry *end() const { return reinterpret_cast<const Entry*>(&m_storage[m_count]); }

private:
	bool Find(const TKey key, size_t& idx)
	{
		for (idx = 0; idx < m_count; idx++)
		{
			if (m_storage[idx].Key == key)
				return true;
		}
		return false;
	}

	size_t m_count;
	Entry m_storage[N];
};
