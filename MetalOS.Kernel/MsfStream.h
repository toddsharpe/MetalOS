#pragma once

#include "msvc.h"
#include <cstdint>
#include <vector>
#include "MsfFile.h"
#include <algorithm>
#include <string>

//This doesn't do any caching since PDB is in memory
//m_blockNumbers could have been a pointer to the array since all of the PDB is in memory at once
//but technically these block numbers could be in blocks that aren't contiguous 
class MsfStream
{
public:
	MsfStream(const std::vector<uint32_t>& blockNumbers, const uint32_t size, const MsfFile& file);

	template<typename T>
	T Read()
	{
		static_assert(sizeof(T) <= 8, "Invalid T");
		
		const size_t blockNumber = m_position / m_file.BlockSize();
		const size_t blockIndex = m_position % m_file.BlockSize();

		//Assert T doesn't span pages
		//Assert(((m_position + sizeof(T) - 1) / PAGE_SIZE) == blockNumber);

		const void* block = m_file.GetBlock(m_blockNumbers[blockNumber]);
		const T value = *(T*)((uintptr_t)block + blockIndex);

		m_position += sizeof(T);

		return value;
	}

	bool ReadString(std::string& value);
	void Read(char* buffer, size_t length);
	bool ReadString(char* str);
	void SkipString();

	template<typename T>
	bool Read(T* value)
	{
		const size_t blockNumber = m_position / m_file.BlockSize();
		const size_t blockIndex = m_position % m_file.BlockSize();

		//Assert T doesn't span pages
		//if (((m_position + sizeof(T) - 1) / m_file.BlockSize()) != blockNumber)
			//return false;

		const void* block = m_file.GetBlock(m_blockNumbers[blockNumber]);
		const void* blockPointer = (void*)((uintptr_t)block + blockIndex);
		const size_t bytes = std::min(sizeof(T), m_file.BlockSize() - blockIndex);
		memcpy(value, blockPointer, bytes);

		const size_t remaining = sizeof(T) - bytes;
		if (remaining != 0)
		{
			const void* nextBlock = m_file.GetBlock(m_blockNumbers[blockNumber + 1]);
			void* valuePointer = (void*)((uintptr_t)value + bytes);
			memcpy(valuePointer, nextBlock, remaining);
		}

		m_position += sizeof(T);

		return true;
	}

	#define AlignSize(x,a) (((x) + ((a) - 1)) & ~((a)-1))
	template<typename T>
	void SkipAlign()
	{
		m_position = AlignSize(m_position, sizeof(T));
	}

	template<typename T>
	void Skip(const uint32_t n)
	{
		m_position += n * sizeof(T);
	}

	void Skip(const uint32_t n)
	{
		m_position += n;
	}

	void Advance(const uint32_t position)
	{
		m_position += position;
	}

	void Seek(const uint32_t position)
	{
		m_position = position;
	}

	uint32_t Position() const
	{
		return m_position;
	}

	void Display() const;

	uint32_t GetSize() const
	{
		return m_size;
	}

private:
	void* GetPointer() const;

	std::vector<uint32_t> m_blockNumbers;
	const uint32_t m_size;
	const MsfFile& m_file;

	//State
	uint32_t m_position;
};

