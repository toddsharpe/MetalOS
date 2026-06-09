#pragma once

#include <cstdint>
#include <cstring>

template <typename T, size_t N>
class CircularBuffer
{
public:
	constexpr CircularBuffer() :
		m_head(),
		m_tail(),
		m_count(),
		m_storage()
	{

	}

	bool Enqueue(const T value)
	{
		if (IsFull())
			return false;

		m_storage[m_tail] = value;
		m_tail = (m_tail + 1) % N;
		m_count++;

		return true;
	}

	T& Peek()
	{
		Assert(!IsEmpty());

		return m_storage[m_head];
	}

	T Dequeue()
	{
		Assert(!IsEmpty());

		T item = m_storage[m_head];
		m_head = (m_head + 1) % N;
		m_count--;

		return item;
	}

	size_t Count() const
	{
		return m_count;
	}

	bool IsEmpty() const
	{
		return m_count == 0;
	}

	bool IsFull() const
	{
		return m_count == N;
	}

private:
	size_t m_head;
	size_t m_tail;
	size_t m_count;
	T m_storage[N];
};
