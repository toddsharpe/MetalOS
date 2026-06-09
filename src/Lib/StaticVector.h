#pragma once

#include <cstdint>
#include <cstring>

template <typename T, size_t N>
class StaticVector
{
public:
	constexpr StaticVector() :
		m_storage(),
		m_count()
	{
	}

	bool Add(const T value)
	{
		if (m_count == N)
			return false;

		m_storage[m_count] = value;
		m_count++;
		return true;
	}

	bool Remove(const T value)
	{
		size_t index = 0;
		if (!Find(value, index))
			return false;

		return RemoveAt(index);
	}

	bool RemoveAt(const size_t index)
	{
		Assert(index < m_count);

		if (index < N - 1)
		{
			//Shift items down
			const size_t num = m_count - index - 1;
			memcpy(&m_storage[index], &m_storage[index + 1], num * sizeof(T));
		}

		m_count--;
		return true;
	}

	bool Contains(const T value) const
	{
		size_t index = 0;
		return Find(value, index);
	}

	T First() const
	{
		Assert(m_count > 0);
		return m_storage[0];
	}

	T operator[](const size_t index) const
	{
		Assert(index < m_count);
		return m_storage[index];
	}

	size_t Count() const
	{
		return m_count;
	}

	const T *begin() const { return reinterpret_cast<const T*>(m_storage); }
	const T *end() const { return reinterpret_cast<const T*>(&m_storage[m_count]); }

	T *begin() { return reinterpret_cast<T*>(m_storage); }
	T *end() { return reinterpret_cast<T*>(&m_storage[m_count]); }

private:
	bool Find(const T value, size_t& idx) const
	{
		for (idx = 0; idx < m_count; idx++)
		{
			if (m_storage[idx] == value)
				return true;
		}
		return false;
	}

	T m_storage[N];
	size_t m_count;
};
