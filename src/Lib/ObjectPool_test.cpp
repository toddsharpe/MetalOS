#include "Lib/ObjectPool.h"

void ObjectPool_test()
{
	// Basic allocate and free — slot is zeroed on Deallocate
	{
		ObjectPool<uint32_t, 4> pool;
		uint32_t* a = pool.Allocate();
		Assert(a);
		*a = 0xDEAD;
		pool.Deallocate(a);
		uint32_t* b = pool.Allocate();
		Assert(b == a);
		AssertEqual(*b, 0u);
	}

	// Fill to capacity, reject when full
	{
		ObjectPool<uint32_t, 3> pool;
		uint32_t* a = pool.Allocate();
		uint32_t* b = pool.Allocate();
		uint32_t* c = pool.Allocate();
		Assert(a && b && c);
		Assert(!pool.Allocate());
	}

	// Deallocate frees slot for re-use
	{
		ObjectPool<uint32_t, 2> pool;
		uint32_t* a = pool.Allocate();
		uint32_t* b = pool.Allocate();
		Assert(a && b);
		Assert(!pool.Allocate());
		pool.Deallocate(a);
		uint32_t* c = pool.Allocate();
		Assert(c == a);
		Assert(!pool.Allocate());
	}

	// Pointer arithmetic round-trip: Deallocate finds correct index for every slot.
	// Iterate in reverse so each freed slot is always the lowest free index,
	// guaranteeing Allocate() returns it (FirstClear picks the lowest bit).
	{
		ObjectPool<uint64_t, 8> pool;
		uint64_t* slots[8];
		for (size_t i = 0; i < 8; i++)
			slots[i] = pool.Allocate();

		for (size_t i = 8; i-- > 0; )
		{
			pool.Deallocate(slots[i]);
			uint64_t* reclaimed = pool.Allocate();
			Assert(reclaimed == slots[i]);
			pool.Deallocate(reclaimed);
		}
	}

	// Returned pointers are correctly aligned for T
	{
		struct alignas(16) Wide { uint64_t a, b; };
		ObjectPool<Wide, 4> pool;
		for (size_t i = 0; i < 4; i++)
		{
			Wide* p = pool.Allocate();
			Assert(p);
			Assert((uintptr_t)p % alignof(Wide) == 0);
			pool.Deallocate(p);
		}
	}
}
