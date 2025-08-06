#include "Lib/StaticMap.h"
#include "Assert.h"

void StaticMap_test()
{
	// Basic add and get
	{
		StaticMap<int, int, 4> m;
		Assert(m.Count() == 0);
		Assert(m.Add(1, 100));
		Assert(m.Add(2, 200));
		AssertEqual(m.Count(), 2);
		AssertEqual(m.Get(1), 100);
		AssertEqual(m.Get(2), 200);
	}

	// Get with out-param
	{
		StaticMap<int, int, 4> m;
		m.Add(10, 42);
		int value = 0;
		Assert(m.Get(10, value));
		AssertEqual(value, 42);
	}

	// Get missing key returns 0 / false
	{
		StaticMap<int, int, 4> m;
		m.Add(1, 99);
		AssertEqual(m.Get(99), 0);
		int value = 0;
		Assert(!m.Get(99, value));
	}

	// Fill to capacity, reject when full
	{
		StaticMap<int, int, 3> m;
		Assert(m.Add(1, 1));
		Assert(m.Add(2, 2));
		Assert(m.Add(3, 3));
		Assert(!m.Add(4, 4));
		AssertEqual(m.Count(), 3);
	}

	// Remove from middle — remaining entries shift down
	{
		StaticMap<int, int, 4> m;
		m.Add(1, 10); m.Add(2, 20); m.Add(3, 30); m.Add(4, 40);
		Assert(m.Remove(2));
		AssertEqual(m.Count(), 3);
		AssertEqual(m.Get(1), 10);
		AssertEqual(m.Get(3), 30);
		AssertEqual(m.Get(4), 40);
		AssertEqual(m.Get(2), 0);  // gone
	}

	// Remove first entry
	{
		StaticMap<int, int, 4> m;
		m.Add(1, 10); m.Add(2, 20); m.Add(3, 30);
		Assert(m.Remove(1));
		AssertEqual(m.Count(), 2);
		AssertEqual(m.Get(2), 20);
		AssertEqual(m.Get(3), 30);
	}

	// Remove last entry
	{
		StaticMap<int, int, 4> m;
		m.Add(1, 10); m.Add(2, 20); m.Add(3, 30);
		Assert(m.Remove(3));
		AssertEqual(m.Count(), 2);
		AssertEqual(m.Get(1), 10);
		AssertEqual(m.Get(2), 20);
	}

	// Remove non-existent key returns false
	{
		StaticMap<int, int, 4> m;
		m.Add(1, 10);
		Assert(!m.Remove(99));
		AssertEqual(m.Count(), 1);
	}

	// Add after remove — slot is reused correctly
	{
		StaticMap<int, int, 3> m;
		m.Add(1, 10); m.Add(2, 20); m.Add(3, 30);
		m.Remove(2);
		Assert(m.Add(4, 40));
		AssertEqual(m.Count(), 3);
		AssertEqual(m.Get(4), 40);
	}

	// Range-based for loop via begin/end
	{
		StaticMap<int, int, 4> m;
		m.Add(1, 10); m.Add(2, 20); m.Add(3, 30);
		int keys[] = { 1, 2, 3 };
		int vals[] = { 10, 20, 30 };
		size_t i = 0;
		for (const auto& e : m)
		{
			AssertEqual(e.Key, keys[i]);
			AssertEqual(e.Value, vals[i]);
			i++;
		}
		AssertEqual(i, 3);
	}
}
