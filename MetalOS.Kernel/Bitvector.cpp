#include "Bitvector.h"

#include "Main.h"

constexpr size_t Bitvector::GetMapSize(size_t length)
{
	return length / mapBits + ((length % mapBits != 0) ? 1 : 0);
}

constexpr size_t Bitvector::GetMapIndex(size_t index)
{
	return index / mapBits;
}

constexpr size_t Bitvector::GetMapShift(size_t index)
{
	return index % mapBits;
}

Bitvector::Bitvector(size_t length) : m_size(GetMapSize(length)), m_map(new size_t[m_size]())
{

}

void Bitvector::Set(size_t index)
{
	const size_t mapIndex = GetMapIndex(index);
	const size_t mapShift = GetMapShift(index);
	Assert(mapIndex < m_size);
	
	size_t& element = m_map[mapIndex];
	element |= (1LL << mapShift);
}

bool Bitvector::Get(size_t index) const
{
	const size_t mapIndex = GetMapIndex(index);
	const size_t mapShift = GetMapShift(index);
	Assert(mapIndex < m_size);

	size_t& element = m_map[mapIndex];
	return (element & (1LL << mapShift)) != 0;
}

void Bitvector::Clear(size_t index)
{
	const size_t mapIndex = GetMapIndex(index);
	const size_t mapShift = GetMapShift(index);
	Assert(mapIndex < m_size);

	size_t& element = m_map[mapIndex];
	element &= ~(1LL << mapShift);
}
