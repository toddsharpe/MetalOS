#include "Lib/Bitset.h"
#include "Lib/Arena.h"

void Bitset_test()
{
	// StaticBitset: initial state is all clear
	{
		StaticBitset<64> bitset;
		for (size_t i = 0; i < 64; i++)
			Assert(!bitset.Get(i));
	}

	// StaticBitset: Set and Get
	{
		StaticBitset<64> bitset;
		bitset.Set(0);
		Assert(bitset.Get(0));
		Assert(!bitset.Get(1));

		bitset.Set(63);
		Assert(bitset.Get(63));

		bitset.Clear(0);
		Assert(!bitset.Get(0));
		Assert(bitset.Get(63));
	}

	// StaticBitset: FirstClear finds first zero bit
	{
		StaticBitset<8> bitset;
		size_t index = 99;
		Assert(bitset.FirstClear(index));
		AssertEqual(index, 0);

		bitset.Set(0);
		Assert(bitset.FirstClear(index));
		AssertEqual(index, 1);

		// Fill all bits, FirstClear should return false
		for (size_t i = 0; i < 8; i++)
			bitset.Set(i);
		Assert(!bitset.FirstClear(index));
	}

	// StaticBitset: FirstSet finds first set bit
	{
		StaticBitset<8> bitset;
		size_t index = 99;
		Assert(!bitset.FirstSet(index));

		bitset.Set(3);
		Assert(bitset.FirstSet(index));
		AssertEqual(index, 3);

		bitset.Set(1);
		Assert(bitset.FirstSet(index));
		AssertEqual(index, 1);  // Must return lowest index
	}

	// StaticBitset spanning multiple 64-bit chunks (N > 64)
	{
		StaticBitset<128> bitset;
		for (size_t i = 0; i < 128; i++)
			Assert(!bitset.Get(i));

		bitset.Set(64);
		Assert(!bitset.Get(63));
		Assert(bitset.Get(64));
		Assert(!bitset.Get(65));

		bitset.Set(127);
		Assert(bitset.Get(127));
		bitset.Clear(127);
		Assert(!bitset.Get(127));

		size_t index = 99;
		Assert(bitset.FirstSet(index));
		AssertEqual(index, 64);
	}

	// DynamicBitset via arena
	{
		StaticArena<256> arena;
		DynamicBitset bitset(16);
		bitset.Initialize(arena);

		for (size_t i = 0; i < 16; i++)
			Assert(!bitset.Get(i));

		bitset.Set(7);
		Assert(bitset.Get(7));

		size_t index = 99;
		Assert(bitset.FirstSet(index));
		AssertEqual(index, 7);

		bitset.Clear(7);
		Assert(!bitset.FirstSet(index));
	}
}
