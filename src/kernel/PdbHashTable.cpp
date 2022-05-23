#include "PdbHashTable.h"

PdbHashTable::PdbHashTable() :
	m_size(),
	m_capacity(),
	m_vectorLength(),
	m_items()
{

}

void PdbHashTable::Load(MsfStream& stream)
{
	m_size = stream.Read<uint32_t>();
	m_capacity = stream.Read<uint32_t>();
	m_vectorLength = stream.Read<uint32_t>();

	//Skip both Present and Deleted bitvectors
	stream.Skip<uint32_t>(m_vectorLength * 2);

	for (size_t i = 0; i < m_size; i++)
	{
		const uint32_t key = stream.Read<uint32_t>();
		const uint32_t value = stream.Read<uint32_t>();
		m_items.push_back({ key, value });
	}
	stream.SkipAlign<uint16_t>();
}

bool PdbHashTable::GetValue(const uint32_t key, uint32_t& value) const
{
	for (const auto& item : m_items)
	{
		if (key == item.first)
		{
			value = item.second;
			return true;
		}
	}
	return false;
}
