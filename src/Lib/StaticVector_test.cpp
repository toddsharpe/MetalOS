#include "Lib/StaticVector.h"
#include "Assert.h"

void StaticVector_test()
{
	// Basic add and count
	{
		StaticVector<int, 4> v;
		Assert(v.Count() == 0);
		Assert(v.Add(10));
		Assert(v.Add(20));
		Assert(v.Add(30));
		AssertEqual(v.Count(), 3);
		AssertEqual(v[0], 10);
		AssertEqual(v[1], 20);
		AssertEqual(v[2], 30);
	}

	// Fill to capacity, reject when full
	{
		StaticVector<int, 3> v;
		Assert(v.Add(1));
		Assert(v.Add(2));
		Assert(v.Add(3));
		Assert(!v.Add(4));
		AssertEqual(v.Count(), 3);
	}

	// Remove from middle — remaining elements shift down
	{
		StaticVector<int, 4> v;
		v.Add(1); v.Add(2); v.Add(3); v.Add(4);
		Assert(v.Remove(2));
		AssertEqual(v.Count(), 3);
		AssertEqual(v[0], 1);
		AssertEqual(v[1], 3);
		AssertEqual(v[2], 4);
	}

	// Remove first element
	{
		StaticVector<int, 4> v;
		v.Add(1); v.Add(2); v.Add(3);
		Assert(v.Remove(1));
		AssertEqual(v.Count(), 2);
		AssertEqual(v[0], 2);
		AssertEqual(v[1], 3);
	}

	// Remove last element
	{
		StaticVector<int, 4> v;
		v.Add(1); v.Add(2); v.Add(3);
		Assert(v.Remove(3));
		AssertEqual(v.Count(), 2);
		AssertEqual(v[0], 1);
		AssertEqual(v[1], 2);
	}

	// Remove non-existent value returns false
	{
		StaticVector<int, 4> v;
		v.Add(1); v.Add(2);
		Assert(!v.Remove(99));
		AssertEqual(v.Count(), 2);
	}

	// Find does not match uninitialized slots past m_count
	{
		StaticVector<int, 8> v;
		v.Add(1); v.Add(2); v.Add(3);
		// 0 is default-initialized in remaining slots; must not be found
		Assert(!v.Remove(0));
		AssertEqual(v.Count(), 3);
	}

	// RemoveAt by index
	{
		StaticVector<int, 4> v;
		v.Add(10); v.Add(20); v.Add(30);
		Assert(v.RemoveAt(1));
		AssertEqual(v.Count(), 2);
		AssertEqual(v[0], 10);
		AssertEqual(v[1], 30);
	}

	// Range-based for loop via begin/end
	{
		StaticVector<int, 4> v;
		v.Add(5); v.Add(6); v.Add(7);
		int expected[] = { 5, 6, 7 };
		size_t i = 0;
		for (const int val : v)
		{
			AssertEqual(val, expected[i]);
			i++;
		}
		AssertEqual(i, 3);
	}

	// Add after remove — slot is reused correctly
	{
		StaticVector<int, 3> v;
		v.Add(1); v.Add(2); v.Add(3);
		v.Remove(2);
		Assert(v.Add(99));
		AssertEqual(v.Count(), 3);
		AssertEqual(v[2], 99);
	}
}
