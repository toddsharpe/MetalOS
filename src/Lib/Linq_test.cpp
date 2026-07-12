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

static void add_to_sum(const int& item, int* sum)
{
	*sum += item;
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

	// Count — total and empty
	{
		LinkedList<int> list;
		list.Add(1);
		list.Add(2);
		list.Add(3);
		AssertEqual(Linq::Count(list), (size_t)3);

		LinkedList<int> empty;
		AssertEqual(Linq::Count(empty), (size_t)0);
	}

	// Count — by predicate
	{
		LinkedList<int> list;
		list.Add(10);
		list.Add(20);
		list.Add(30);
		AssertEqual(Linq::Count(list, greater_than, 15), (size_t)2);
	}

	// ForEach — visits every item
	{
		LinkedList<int> list;
		list.Add(1);
		list.Add(2);
		list.Add(3);
		list.Add(4);

		int sum = 0;
		Linq::ForEach(list, add_to_sum, &sum);
		AssertEqual(sum, 10);
	}
}
