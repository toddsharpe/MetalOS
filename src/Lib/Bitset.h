#pragma once

#include "core_crt/stdint.h"
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

	bool FirstClear(size_t& index)
	{
		Assert(m_map);
		for (index = 0; index < Length; index++)
		{
			const size_t mapIndex = GetMapIndex(index);
			const size_t mapShift = GetMapShift(index);

			uint64_t element = m_map[mapIndex];
			const uint64_t mask = (1LL << mapShift);
			if ((element & mask) == 0)
				return true;
		}
		return false;
	}

	bool FirstSet(size_t& index)
	{
		Assert(m_map);
		for (index = 0; index < Length; index++)
		{
			const size_t mapIndex = GetMapIndex(index);
			const size_t mapShift = GetMapShift(index);

			uint64_t &element = m_map[mapIndex];
			const uint64_t mask = (1LL << mapShift);
			if ((element & mask) != 0)
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

	template <typename T>
	void Initialize(T &arena)
	{
		const size_t mapSize = DivRoundUp(Length, MapBits);
		const size_t allocSize = mapSize * sizeof(uint64_t);
		m_map = reinterpret_cast<uint64_t *>(arena.Allocate(allocSize));
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

private:
	static constexpr size_t Chunks = DivRoundUp(N, MapBits);
	uint64_t m_storage[Chunks];
};
