#include "Bitvector.h"

#include "Assert.h"

namespace
{
	static constexpr size_t mapBits = std::numeric_limits<size_t>::digits;

	inline size_t GetMapSize(const size_t length)
	{
		return (length + (mapBits - 1)) / mapBits;
	}

	inline size_t GetMapIndex(const size_t index)
	{
		return index / mapBits;
	}

	inline size_t GetMapShift(const size_t index)
	{
		return index % mapBits;
	}
}

Bitvector::Bitvector(const size_t length) :
	Length(length),
	m_map()
{

}

void Bitvector::Initialize()
{
	const size_t mapSize = GetMapSize(Length);
	m_map = new size_t[mapSize];
	Assert(m_map);

	memset(m_map, 0, sizeof(size_t) * mapSize);
}

bool Bitvector::Get(const size_t index) const
{
	Assert(m_map);
	AssertOp(index, <, Length);
	
	const size_t mapIndex = GetMapIndex(index);
	const size_t element = m_map[mapIndex];

	const size_t mapShift = GetMapShift(index);
	const size_t mask = (1LL << mapShift);
	return (element & mask) != 0;
}

void Bitvector::Set(const size_t index, const bool value)
{
	Assert(m_map);
	AssertOp(index, <, Length);
	
	const size_t mapIndex = GetMapIndex(index);
	size_t& const element = m_map[mapIndex];

	const size_t mapShift = GetMapShift(index);
	const size_t mask = (1LL << mapShift);
	
	if (value)
		element |= mask;
	else
		element &= ~mask;
}


