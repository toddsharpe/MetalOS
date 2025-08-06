#pragma once

#include "core_crt/stdint.h"
#include "Assert.h"

template <typename T, size_t N>
class StaticStack
{
public:
	struct ReverseView
	{
	public:
		ReverseView(const StaticStack<T, N> &stack, const size_t index) :
			m_stack(stack),
			m_index(index)
		{
		}

		//Prefix called from range-based for loop
		ReverseView& operator++()
		{
			m_index--;
			return *this;
		}

		const T &operator*()
		{
			return m_stack[m_index];
		}

		bool operator==(const ReverseView& rhs) const
		{
			return &this->m_stack == &rhs.m_stack && this->m_index == rhs.m_index;
		}

		bool operator!=(const ReverseView& rhs) const
		{
			return !(*this == rhs);
		}

	private:
		const StaticStack<T, N> &m_stack;
		size_t m_index = 0;
	};

	StaticStack() : m_storage(), m_count()
	{
	}

	bool Push(const T &item)
	{
		if (m_count == N)
			return false;

		m_storage[m_count] = item;
		m_count++;
		return true;
	}

	T Pop()
	{
		Assert(m_count > 0);
		m_count--;
		return m_storage[m_count];
	}

	bool Evict(const T value)
	{
		size_t index = 0;
		if (!Find(value, index))
			return false;

		return RemoveAt(index);
	}

	const T &operator[](const size_t index) const
	{
		AssertOp(index, <, m_count);
		return m_storage[index];
	}

	// Iterate items from top of stack to bottom
	ReverseView begin() const
	{
		return ReverseView(*this, Count() - 1);
	}

	ReverseView end() const
	{
		return ReverseView(*this, -1);
	}

	size_t Count() const
	{
		return m_count;
	}

	bool IsEmpty() const
	{
		return m_count == 0;
	}

private:
	bool Find(const T value, size_t &idx)
	{
		for (idx = 0; idx < m_count; idx++)
		{
			if (m_storage[idx] == value)
				return true;
		}
		return false;
	}

	bool RemoveAt(const size_t index)
	{
		Assert(index < m_count);

		if (index < N - 1)
		{
			// Shift items down
			const size_t num = m_count - index - 1;
			memcpy(&m_storage[index], &m_storage[index + 1], num * sizeof(T));
		}

		m_count--;
		return true;
	}

	T m_storage[N];
	size_t m_count;
};
