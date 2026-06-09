#include "Lib/Linq.h"
#include "Lib/LinkedList.h"

static bool equals_int(const int& item, int target)
{
	return item == target;
}

static bool greater_than(const int& item, int threshold)
{
	return item > threshold;
}

void Linq_test()
{
	// First — element exists
	{
		LinkedList<int> list;
		list.Add(10);
		list.Add(20);
		list.Add(30);

		const int* result = Linq::First(list, equals_int, 20);
		Assert(result != nullptr);
		AssertEqual(*result, 20);
	}

	// First — no match returns nullptr
	{
		LinkedList<int> list;
		list.Add(10);
		list.Add(20);

		Assert(Linq::First(list, equals_int, 99) == nullptr);
	}

	// First — returns first match, not last
	{
		LinkedList<int> list;
		list.Add(5);
		list.Add(10);
		list.Add(5);

		const int* result = Linq::First(list, equals_int, 5);
		Assert(result != nullptr);
		AssertEqual(*result, 5);
	}

	// First — empty list returns nullptr
	{
		LinkedList<int> empty;
		Assert(Linq::First(empty, equals_int, 1) == nullptr);
	}

	// Any — match exists
	{
		LinkedList<int> list;
		list.Add(10);
		list.Add(20);
		list.Add(30);

		Assert(Linq::Any(list, greater_than, 25));
	}

	// Any — no match
	{
		LinkedList<int> list;
		list.Add(10);
		list.Add(20);

		Assert(!Linq::Any(list, greater_than, 100));
	}

	// Any — empty list returns false
	{
		LinkedList<int> empty;
		Assert(!Linq::Any(empty, equals_int, 1));
	}

	// Any — single element matches
	{
		LinkedList<int> list;
		list.Add(42);

		Assert(Linq::Any(list, equals_int, 42));
		Assert(!Linq::Any(list, equals_int, 0));
	}
}
