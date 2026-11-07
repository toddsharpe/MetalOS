#pragma once

#include <cstddef>
#include <cstdint>
#include "Lib/Span.h"
#include "Assert.h"

//Fixed-size blocks over storage it doesn't own: an allocation is the lowest run of free blocks that holds it
template <size_t N, size_t BlockSize = 256>
class BlockAllocator
{
	static_assert(BlockSize != 0, "A block of no bytes holds nothing");
	static_assert(N >= BlockSize, "The storage must hold at least one block");

public:
	static constexpr size_t BlockCount = N / BlockSize;

	constexpr BlockAllocator(uint8_t (&storage)[N]) :
		m_storage(storage),
		m_used()
	{

	}

	BlockAllocator(const BlockAllocator&) = delete;
	BlockAllocator& operator=(const BlockAllocator&) = delete;

	//Empty when no run of free blocks is long enough
	buffer_t Allocate(const size_t size)
	{
		const size_t blocks = BlocksFor(size);
		if (blocks == 0 || blocks > BlockCount)
			return {};

		const size_t first = Find(blocks);
		if (first == BlockCount)
			return {};

		for (size_t i = first; i < first + blocks; i++)
			m_used[i] = true;

		return { &m_storage[first * BlockSize], size };
	}

	//Freeing an empty buffer does nothing, so a failed Allocate needs no check before it
	void Free(const buffer_t allocation)
	{
		if (!allocation)
			return;

		Assert(allocation.Data >= m_storage);
		const size_t offset = static_cast<size_t>(allocation.Data - m_storage);
		Assert(offset % BlockSize == 0);

		const size_t first = offset / BlockSize;
		const size_t blocks = BlocksFor(allocation.Size);
		Assert(first + blocks <= BlockCount);

		for (size_t i = first; i < first + blocks; i++)
		{
			Assert(m_used[i]);
			m_used[i] = false;
		}
	}

private:
	//The first block of the lowest run of free ones, or BlockCount when there is none
	size_t Find(const size_t blocks) const
	{
		size_t run = 0;
		for (size_t i = 0; i < BlockCount; i++)
		{
			if (m_used[i])
			{
				run = 0;
				continue;
			}

			run++;
			if (run == blocks)
				return i + 1 - blocks;
		}

		return BlockCount;
	}

	static constexpr size_t BlocksFor(const size_t size)
	{
		return (size + BlockSize - 1) / BlockSize;
	}

	uint8_t* m_storage;
	bool m_used[BlockCount];
};
