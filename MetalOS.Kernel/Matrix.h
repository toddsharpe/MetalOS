#pragma once
#include <cstdint>
#include <MetalOS.h>
#include "Main.h"

//How doesn't C++ have a way to do contiguous multi-dimensional arrays?
template<class T>
class Matrix
{
public:
	Matrix(const size_t M, const size_t N) :
		m_height(M),
		m_width(N),
		m_backing(new T[N * M * sizeof(T)])
	{
		Assert(m_backing);
	}

	void Set(const Point2D p, const T value) const
	{
		const size_t index = GetIndex(p);
		m_backing[index] = value;
	}

	T Get(const Point2D p) const
	{
		const size_t index = GetIndex(p);
		return m_backing[index];
	}

	size_t GetHeight() const
	{
		return m_height;
	}

	size_t GetWidth() const
	{
		return m_width;
	}

	T* const Buffer() const
	{
		return m_backing;
	}

	size_t Size() const
	{
		return m_height * m_width * sizeof(T);
	}

private:
	constexpr size_t GetIndex(const Point2D p) const
	{
		Assert(p.Y < m_height);
		Assert(p.X < m_width);
		return p.Y * m_width + p.X;
	}

	const size_t m_height;
	const size_t m_width;
	T* const m_backing;
};
