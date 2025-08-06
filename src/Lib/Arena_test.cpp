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
}
