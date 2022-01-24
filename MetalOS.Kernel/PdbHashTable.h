#pragma once
#include "MsfStream.h"


//https://llvm.org/docs/PDB/HashTable.html
//TODO: template for key/value length
//rounded to nearest 16 bytes?
class PdbHashTable
{
public:
	PdbHashTable();
	void Load(MsfStream& stream);
	bool GetValue(const uint32_t key, uint32_t& value) const;

private:
	/*
	struct Table
	{
		uint32_t Size;
		uint32_t Capacity;
		uint32_t BitVectorSize;
		uint32_t Present[BitVectorSize];
		uint32_t Deleted[BitVectorSize];
		struct
		{
			uint32_t Key;
			TValue Value;
		} Buckets[Size]; - only present buckets are written to disk
	};
	*/

	uint32_t m_size;
	uint32_t m_capacity;
	uint32_t m_vectorLength;
	std::vector<std::pair<uint32_t, uint32_t>> m_items;
};
