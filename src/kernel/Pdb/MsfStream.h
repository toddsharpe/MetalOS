#pragma once

#include "Lib/StaticVector.h"
#include "kernel/Pdb/Platform.h"
#include "Assert.h"

//This doesn't do any caching since PDB is in memory
//BlockNumbers is saved since blocks could be non-contiguous
class MsfStream
{
public:
	MsfStream(const size_t blockSize, const ReadBlock read, void* const context) :
		Size(),
		IsInvalid(),
		BlockCount(),
		BlockNumbers(),
		Link(),
		m_blockSize(blockSize),
		m_read(read),
		m_context(context),
		m_position()
	{

	}

	template<typename T>
	T Read()
	{
		static_assert(sizeof(T) <= 8, "Invalid T");
		
		const size_t blockNumber = m_position / m_blockSize;
		const size_t blockIndex = m_position % m_blockSize;

		//Assert T doesn't span pages
		//Assert(((m_position + sizeof(T) - 1) / PageSize) == blockNumber);

		const void* block = m_read(m_context, BlockNumbers[blockNumber]);
		const T value = *(T*)((uintptr_t)block + blockIndex);

		m_position += sizeof(T);
		return value;
	}

	bool ReadString(StringStorage& value)
	{
		char* blockPointer = (char*)GetPointer();
		while ((*blockPointer) != '\0')
		{
			value.Append(*blockPointer);

			m_position++;
			blockPointer = (char*)GetPointer();
		}

		m_position++;
		return true;
	}

	void Read(char* buffer, size_t length)
	{
		char* blockPointer = (char*)GetPointer();
		for (size_t i = 0; i < length; i++)
		{
			*buffer = *blockPointer;

			buffer++;
			m_position++;
			blockPointer = (char*)GetPointer();
		}
	}

	bool ReadString(char* str)
	{
		char* blockPointer = (char*)GetPointer();
		while ((*blockPointer) != '\0')
		{
			*str = *blockPointer;

			m_position++;
			str++;
			blockPointer = (char*)GetPointer();
		}
		*str = '\0';

		m_position++;
		return true;
	}

	void SkipString()
	{
		char* blockPointer = (char*)GetPointer();
		while ((*blockPointer) != '\0')
		{
			m_position++;
			blockPointer = (char*)GetPointer();
		}

		m_position++;
	}

	template<typename T>
	bool Read(T* value)
	{
		const size_t blockNumber = m_position / m_blockSize;
		const size_t blockIndex = m_position % m_blockSize;

		//Assert T doesn't span pages
		//if (((m_position + sizeof(T) - 1) / m_blockSize) != blockNumber)
			//return false;

		const void* block = m_read(m_context, BlockNumbers[blockNumber]);
		const void* blockPointer = (void*)((uintptr_t)block + blockIndex);
		const size_t bytes = Min(sizeof(T), m_blockSize - blockIndex);
		memcpy(value, blockPointer, bytes);

		const size_t remaining = sizeof(T) - bytes;
		if (remaining != 0)
		{
			const void* nextBlock = m_read(m_context, BlockNumbers[blockNumber + 1]);
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

	void Display() const
	{
		if (BlockNumbers.Count() == 0)
			Printf("Empty stream\n");
		else
		{
			Printf("Start: 0x%x End:0x%x Size: 0x%x, Pos: 0x%08x\n", BlockNumbers[0], BlockNumbers[BlockNumbers.Count() - 1], Size, Position());

			for (size_t j = 0; j < BlockNumbers.Count(); j++)
			{
				Printf("%x,", BlockNumbers[j]);
				if (j != 0 && j % 16 == 0)
					Printf("\n");
			}
			Printf("\n");
		}
	}

	uint32_t Size;
	bool IsInvalid;
	size_t BlockCount;
	StaticVector<uint32_t, 512> BlockNumbers;
	ListEntry Link;

private:
	void* GetPointer() const
	{
		const size_t blockNumber = m_position / m_blockSize;
		const size_t blockIndex = m_position % m_blockSize;

		const void* block = m_read(m_context, BlockNumbers[blockNumber]);
		return (void*)((uintptr_t)block + blockIndex);
	}

	const size_t m_blockSize;
	const ReadBlock m_read;
	void* const m_context;

	//State
	uint32_t m_position;
};
