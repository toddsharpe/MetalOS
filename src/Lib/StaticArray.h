#pragma once

#include "Assert.h"
#include <initializer_list>

template <typename T, size_t N>
class StaticArray
{
public:
	constexpr StaticArray() :
		m_storage()
	{
	}

	constexpr StaticArray(std::initializer_list<T> init) :
		m_storage()
	{
		Assert(init.size() <= N);
		size_t i = 0;
		for (const T& item : init)
			m_storage[i++] = item;
	}

	T operator[](const size_t index) const
	{
		Assert(index < N);
		return m_storage[index];
	}

	const T *begin() const { return reinterpret_cast<const T*>(m_storage); }
	const T *end() const { return reinterpret_cast<const T*>(&m_storage[N]); }

	T *begin() { return reinterpret_cast<T*>(m_storage); }
	T *end() { return reinterpret_cast<T*>(&m_storage[N]); }

	static constexpr size_t Size = N;

private:
	T m_storage[N];
};
