#pragma once

#include "Lib/StaticMap.h"
#include "MsfStream.h"

//https://llvm.org/docs/PDB/HashTable.html
//TODO: template for key/value length
//rounded to nearest 16 bytes?
class PdbHashTable
{
public:
	PdbHashTable() :
		m_size(),
		m_capacity(),
		m_vectorLength(),
		m_items()
	{

	}

	void Load(MsfStream& stream)
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
			m_items.Add(key, value );
		}
		stream.SkipAlign<uint16_t>();
	}

	bool GetValue(const uint32_t key, uint32_t& value) const
	{
		return m_items.Get(key, value);
	}

private:
	uint32_t m_size;
	uint32_t m_capacity;
	uint32_t m_vectorLength;
	StaticMap<uint32_t, uint32_t, 512> m_items;
};
