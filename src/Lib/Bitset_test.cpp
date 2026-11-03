#include "Lib/Bitset.h"
#include <cstring>

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

		bitset.Set(0, false);
		Assert(!bitset.Get(0));
		Assert(bitset.Get(63));
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
		bitset.Set(127, false);
		Assert(!bitset.Get(127));
	}

	// FirstContiguous: all clear, find 1
	{
		StaticBitset<8> bitset;
		size_t index = 99;
		Assert(bitset.FirstContiguous(1, index));
		AssertEqual(index, 0);
	}

	// FirstContiguous: all clear, find N at start
	{
		StaticBitset<8> bitset;
		size_t index = 99;
		Assert(bitset.FirstContiguous(4, index));
		AssertEqual(index, 0);
	}

	// FirstContiguous: skip leading set bits, run in middle
	{
		StaticBitset<8> bitset;
		bitset.Set(0);
		bitset.Set(1);
		// bits 2..7 are clear
		size_t index = 99;
		Assert(bitset.FirstContiguous(3, index));
		AssertEqual(index, 2);
	}

	// FirstContiguous: run surrounded by set bits
	{
		StaticBitset<8> bitset;
		bitset.Set(0);
		bitset.Set(1);
		bitset.Set(2);
		bitset.Set(6);
		bitset.Set(7);
		// clear run at [3,4,5]
		size_t index = 99;
		Assert(bitset.FirstContiguous(3, index));
		AssertEqual(index, 3);
	}

	// FirstContiguous: run at the very end (off-by-one check)
	{
		StaticBitset<8> bitset;
		for (size_t i = 0; i < 5; i++)
			bitset.Set(i);
		// bits 5,6,7 are clear
		size_t index = 99;
		Assert(bitset.FirstContiguous(3, index));
		AssertEqual(index, 5);
	}

	// FirstContiguous: no run long enough
	{
		StaticBitset<8> bitset;
		bitset.Set(2);
		bitset.Set(5);
		// clear runs: [0,1], [3,4], [6,7] — each 2, not 3
		size_t index = 99;
		Assert(!bitset.FirstContiguous(3, index));
	}

	// FirstContiguous: all set, looking for clear → false
	{
		StaticBitset<8> bitset;
		for (size_t i = 0; i < 8; i++)
			bitset.Set(i);
		size_t index = 99;
		Assert(!bitset.FirstContiguous(1, index));
	}

	// FirstContiguous: startIdx skips earlier match
	{
		StaticBitset<8> bitset;
		// bits 0..3 clear, bits 4,5 set, bits 6,7 clear
		bitset.Set(4);
		bitset.Set(5);
		size_t index = 99;
		Assert(bitset.FirstContiguous(2, index, false, 6));
		AssertEqual(index, 6);
	}

	// FirstContiguous: state=true, find run of set bits
	{
		StaticBitset<8> bitset;
		bitset.Set(3);
		bitset.Set(4);
		bitset.Set(5);
		size_t index = 99;
		Assert(bitset.FirstContiguous(3, index, true));
		AssertEqual(index, 3);
	}
}
