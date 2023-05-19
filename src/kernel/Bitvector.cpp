#include "Bitvector.h"

#include "Assert.h"

Bitvector::Bitvector(const size_t length) :
	Length(length),
	m_map()
{
	const size_t mapSize = GetMapSize(length);
	m_map.reset(new size_t[mapSize]);

	memset(m_map.get(), 0, sizeof(size_t) * mapSize);
}

bool Bitvector::Get(const size_t index) const
{
	AssertOp(index, <, Length);
	
	const size_t mapIndex = GetMapIndex(index);
	const size_t element = m_map.get()[mapIndex];

	const size_t mapShift = GetMapShift(index);
	const size_t mask = (1LL << mapShift);
	return (element & mask) != 0;
}

void Bitvector::Set(const size_t index, const bool value)
{
	AssertOp(index, <, Length);
	
	const size_t mapIndex = GetMapIndex(index);
	size_t& const element = m_map.get()[mapIndex];

	const size_t mapShift = GetMapShift(index);
	const size_t mask = (1LL << mapShift);
	
	if (value)
		element |= mask;
	else
		element &= ~mask;
}

constexpr size_t Bitvector::GetMapSize(size_t length)
{
	return (length + (mapBits - 1)) / mapBits;
}

constexpr size_t Bitvector::GetMapIndex(size_t index)
{
	return index / mapBits;
}

constexpr size_t Bitvector::GetMapShift(size_t index)
{
	return index % mapBits;
}
