#include "Lib/Arena.h"

struct test_struct
{
	test_struct() : v(), c()
	{
	}

	uint32_t v;
	char c;
};

struct test_struct2
{
	test_struct2(const uint32_t v, const char c) : m_v(v), m_c(c)
	{
	}

	uint32_t m_v;
	char m_c;
};

void Arena_test()
{
	StaticArena<128> arena;
	void *const space = arena.Allocate(16);
	Assert(space);

	void* const space2 = arena.Allocate<uint32_t>();
	AssertEqual((uintptr_t)space2 - (uintptr_t)space, 16);

	uint64_t *i = arena.Allocate<uint64_t>();
	Assert(i);

	test_struct *t = arena.Allocate<test_struct>();
	Assert(t);

	test_struct2 *t2 = arena.Allocate<test_struct2>(1, 'a');
	Assert(t);
	AssertEqual(t2->m_v, 1);
	AssertEqual(t2->m_c, 'a');

	uint8_t storage[32];
	PreallocatedArena prealloc(storage);

	uint8_t* buffer = (uint8_t*)storage;
	PreallocatedArena prealloc2(buffer, sizeof(storage));

	const CString def = "def";
	String new_s;
	new_s = arena.Copy(def);
	Assert(new_s == "def");

	//String copy
	{
		CString copy = arena.Copy({"source", 6});
		Assert(copy == "source");
	}

	//Bounds are checked against the aligned size, so nothing past the arena is written
	{
		uint8_t backing[64];
		memset(backing, 0xAA, sizeof(backing));
		PreallocatedArena bounded(backing, 40);

		Assert(bounded.Allocate(16));
		Assert(bounded.Allocate(16));
		Assert(!bounded.Allocate(1));
		Assert(!bounded.Reserve(1));
		for (size_t i = 32; i < sizeof(backing); i++)
			AssertEqual(backing[i], 0xAA);

		bounded.Reset();
		Assert(bounded.Reserve(40 - 8));
		Assert(!bounded.Reserve(8));
	}

	//An exact fit succeeds and the next allocation fails
	{
		uint8_t backing[48];
		PreallocatedArena exact(backing);
		Assert(exact.Allocate(16));
		Assert(exact.Allocate(17));
		Assert(!exact.Allocate(1));
	}

	//Sizes whose alignment wraps are rejected
	{
		StaticArena<64> wrap;
		Assert(!wrap.Allocate(SIZE_MAX));
		Assert(!wrap.Reserve(SIZE_MAX - 1));
		Assert(wrap.Allocate(64));
	}
}
