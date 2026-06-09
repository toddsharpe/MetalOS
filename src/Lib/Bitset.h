#pragma once

#include <cstdint>
#include <cstdlib>
#include "Lib/Math.h"
#include "Assert.h"

class Bitset
{
public:
	constexpr Bitset(uint64_t *const map, const size_t length) :
		m_map(map),
		Length(length)
	{
	}

	bool Get(const size_t index) const
	{
		Assert(m_map);
		AssertOp(index, <, Length);

		const size_t mapIndex = GetMapIndex(index);
		const uint64_t element = m_map[mapIndex];

		const size_t mapShift = GetMapShift(index);
		const uint64_t mask = (1LL << mapShift);
		return (element & mask) != 0;
	}

	void Clear(const size_t index)
	{
		Set(index, false);
	}

	bool First(size_t& index, const bool state = false, const size_t startIdx = 0) const
	{
		Assert(m_map);
		for (index = startIdx; index < Length; index++)
		{
			if (Get(index) == state)
				return true;
		}
		return false;
	}

	bool FirstContiguous(const size_t count, size_t& index, const bool state = false, const size_t startIdx = 0)
	{
		Assert(m_map);

		for (index = startIdx; index + count <= Length; index++)
		{
			bool found = true;
			for (size_t i = 0; i < count; i++)
			{
				if (Get(index + i) != state)
				{
					found = false;
					break;
				}
			}
			if (found)
				return true;
		}

		return false;
	}

	void Set(const size_t index, const bool value = true)
	{
		Assert(m_map);
		AssertOp(index, <, Length);

		const size_t mapIndex = GetMapIndex(index);
		uint64_t &element = m_map[mapIndex];

		const size_t mapShift = GetMapShift(index);
		const uint64_t mask = (1LL << mapShift);

		if (value)
			element |= mask;
		else
			element &= ~mask;
	}

	void Display() const
	{
		Assert(m_map);
		for (size_t i = 0; i < Length / MapBits; i++)
		{
			Printf("%016x ", m_map[i]);
		}
	}

	size_t const Length;

protected:
	static constexpr size_t MapBits = 64;
	uint64_t *m_map;

private:
	static constexpr size_t GetMapIndex(const size_t index)
	{
		return index / MapBits;
	}

	static constexpr size_t GetMapShift(const size_t index)
	{
		return index % MapBits;
	}
};

class DynamicBitset : public Bitset
{
public:
	constexpr DynamicBitset(const size_t length) :
		Bitset(nullptr, length)
	{
	}

	void Initialize()
	{
		const size_t mapSize = DivRoundUp(Length, MapBits);
		const size_t allocSize = mapSize * sizeof(uint64_t);
		m_map = reinterpret_cast<uint64_t *>(malloc(allocSize));
		Assert(m_map);

		// Zero out region
		memset(m_map, 0, allocSize);
	}
};

// Fixed number of elements
template <size_t N>
class StaticBitset : public Bitset
{
public:
	constexpr StaticBitset() :
		Bitset(m_storage, N),
		m_storage()
	{

	}

	void Initialize()
	{
		//Do nothing
	}

private:
	static constexpr size_t Chunks = DivRoundUp(N, MapBits);
	uint64_t m_storage[Chunks];
};
