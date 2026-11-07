#include "Lib/BlockAllocator.h"

namespace
{
	constexpr size_t Block = 16;
	constexpr size_t Pool = 64;
}

void BlockAllocator_test()
{
	//A partial last block holds nothing
	static_assert(BlockAllocator<Pool, Block>::BlockCount == 4);
	static_assert(BlockAllocator<70, Block>::BlockCount == 4);
	static_assert(BlockAllocator<Block, Block>::BlockCount == 1);

	//Allocations come from the front, a block at a time, and keep the size asked for
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		const buffer_t first = allocator.Allocate(Block);
		Assert(first.Data == &storage[0]);
		AssertEqual(first.Size, Block);

		const buffer_t partial = allocator.Allocate(1);
		Assert(partial.Data == &storage[Block]);
		AssertEqual(partial.Size, static_cast<size_t>(1));
	}

	//A full pool refuses even a byte
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		for (size_t i = 0; i < 4; i++)
			Assert(allocator.Allocate(Block).Data == &storage[i * Block]);

		Assert(!allocator.Allocate(1));
	}

	//A size past a block takes a run of them
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		const buffer_t run = allocator.Allocate(Block + 1);
		Assert(run.Data == &storage[0]);
		AssertEqual(run.Size, Block + 1);

		Assert(allocator.Allocate(Block).Data == &storage[2 * Block]);
	}

	//Everything freed makes the whole pool one run again
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		buffer_t taken[4] = {};
		for (size_t i = 0; i < 4; i++)
			taken[i] = allocator.Allocate(Block);
		Assert(!allocator.Allocate(1));

		for (size_t i = 0; i < 4; i++)
			allocator.Free(taken[i]);

		const buffer_t again = allocator.Allocate(Pool);
		Assert(again.Data == &storage[0]);
		AssertEqual(again.Size, Pool);
	}

	//A freed hole holds only what fits in it
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		allocator.Allocate(Block);
		const buffer_t b = allocator.Allocate(Block);
		allocator.Allocate(Block);
		allocator.Free(b);

		Assert(!allocator.Allocate(Block * 2));
		Assert(allocator.Allocate(Block).Data == &storage[Block]);
		Assert(allocator.Allocate(Block).Data == &storage[3 * Block]);
		Assert(!allocator.Allocate(1));
	}

	//A freed run is reused from its start
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		const buffer_t run = allocator.Allocate(Block * 2);
		allocator.Allocate(Block * 2);
		Assert(!allocator.Allocate(1));

		allocator.Free(run);
		Assert(allocator.Allocate(Block * 2).Data == &storage[0]);
	}

	//Nothing and more than the pool are refused; the whole pool is not
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		Assert(!allocator.Allocate(0));
		Assert(!allocator.Allocate(Pool + 1));
		Assert(allocator.Allocate(Pool).Data == &storage[0]);
	}

	//Freeing an empty buffer gives nothing back
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		const buffer_t taken = allocator.Allocate(Pool);
		allocator.Free({});
		Assert(!allocator.Allocate(1));

		allocator.Free(taken);
		Assert(allocator.Allocate(1));
	}

	//An allocation is the storage itself
	{
		uint8_t storage[Pool] = {};
		BlockAllocator<Pool, Block> allocator(storage);

		const buffer_t taken = allocator.Allocate(Block);
		taken[0] = 0xAB;
		taken[Block - 1] = 0xCD;
		AssertEqual(storage[0], static_cast<uint8_t>(0xAB));
		AssertEqual(storage[Block - 1], static_cast<uint8_t>(0xCD));
	}
}
