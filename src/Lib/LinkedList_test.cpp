#include "Lib/LinkedList.h"

void LinkedList_test()
{
	// Count starts at zero
	{
		LinkedList<int> list;
		AssertEqual(list.Count(), (size_t)0);
		Assert(list.IsEmpty());
	}

	// Add, Count, insertion order preserved
	{
		LinkedList<int> list;
		list.Add(10);
		list.Add(20);
		list.Add(30);
		AssertEqual(list.Count(), (size_t)3);

		int expected[] = {10, 20, 30};
		int i = 0;
		for (int v : list)
			AssertEqual(v, expected[i++]);
		AssertEqual(i, 3);
	}

	// First and Last
	{
		LinkedList<int> list;
		list.Add(1);
		list.Add(2);
		list.Add(3);
		AssertEqual(list.First(), 1);
		AssertEqual(list.Last(), 3);
	}

	// Contains
	{
		LinkedList<int> list;
		list.Add(5);
		list.Add(10);
		Assert(list.Contains(5));
		Assert(list.Contains(10));
		Assert(!list.Contains(99));
	}

	// Add returns true on success
	{
		LinkedList<int> list;
		Assert(list.Add(42));
		AssertEqual(list.First(), 42);
	}

	// Add pre-constructed item
	{
		LinkedList<int> list;
		int x = 99;
		Assert(list.Add(x));
		AssertEqual(list.First(), 99);
	}

	// Remove middle element
	{
		LinkedList<int> list;
		list.Add(1);
		list.Add(2);
		list.Add(3);
		Assert(list.Remove(2));
		AssertEqual(list.Count(), (size_t)2);
		Assert(!list.Contains(2));
		Assert(list.Contains(1));
		Assert(list.Contains(3));
	}

	// Remove head
	{
		LinkedList<int> list;
		list.Add(1);
		list.Add(2);
		Assert(list.Remove(1));
		AssertEqual(list.First(), 2);
		AssertEqual(list.Count(), (size_t)1);
	}

	// Remove tail
	{
		LinkedList<int> list;
		list.Add(1);
		list.Add(2);
		Assert(list.Remove(2));
		AssertEqual(list.Last(), 1);
		AssertEqual(list.Count(), (size_t)1);
	}

	// Remove non-existent value
	{
		LinkedList<int> list;
		list.Add(1);
		Assert(!list.Remove(99));
		AssertEqual(list.Count(), (size_t)1);
	}

	// Remove then re-add
	{
		LinkedList<int> list;
		list.Add(1);
		list.Add(2);
		list.Remove(1);
		list.Add(3);
		AssertEqual(list.Count(), (size_t)2);
		Assert(list.Contains(2));
		Assert(list.Contains(3));
	}

	// Struct uses value comparison via operator==
	{
		struct Tag
		{
			int Id;
			bool operator==(const Tag& rhs) const { return Id == rhs.Id; }
		};
		LinkedList<Tag> list;
		list.Add(Tag{1});
		list.Add(Tag{2});
		Assert(list.Remove(Tag{2}));
		AssertEqual(list.Count(), (size_t)1);
		AssertEqual(list.First().Id, 1);
	}

	// Struct — copy with same fields matches (value comparison)
	{
		struct Tag
		{
			int Id;
			bool operator==(const Tag& rhs) const { return Id == rhs.Id; }
		};
		LinkedList<Tag> list;
		list.Add(Tag{42});
		Tag other{42};
		Assert(list.Remove(other));
		AssertEqual(list.Count(), (size_t)0);
	}

	// Non-trivial type added by value
	{
		struct Point
		{
			int X, Y;
			Point() : X(0), Y(0) {}
			Point(int x, int y) : X(x), Y(y) {}
			bool operator==(const Point& rhs) const { return X == rhs.X && Y == rhs.Y; }
		};

		LinkedList<Point> list;
		Assert(list.Add(Point(3, 4)));
		Assert(list.Contains(Point(3, 4)));
		AssertEqual(list.First().X, 3);
		AssertEqual(list.First().Y, 4);
	}
}
