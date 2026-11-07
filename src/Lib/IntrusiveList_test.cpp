#include "Lib/IntrusiveList.h"

struct IntrusiveItem : ListLink
{
	IntrusiveItem(const int value) : Value(value)
	{

	}

	int Value;
};

void IntrusiveList_test()
{
	// A zero-initialized list is empty and iterates nothing, either way
	{
		IntrusiveList<IntrusiveItem> list;
		Assert(list.IsEmpty());
		for (const IntrusiveItem& item : list)
			Assert(item.Value < 0);
		for (const IntrusiveItem& item : list.Reversed())
			Assert(item.Value < 0);
	}

	// Pop drops the count, so the list reports empty once drained
	{
		IntrusiveList<IntrusiveItem> list;
		IntrusiveItem a(1);
		IntrusiveItem b(2);
		list.InsertHead(a);
		list.InsertHead(b);
		AssertEqual(list.Count(), (size_t)2);

		AssertEqual(list.Pop().Value, 2);
		AssertEqual(list.Count(), (size_t)1);
		AssertEqual(list.Pop().Value, 1);
		Assert(list.IsEmpty());
		Assert(!a.IsLinked() && !b.IsLinked());
	}

	// Remove and InsertTail keep order and count, forwards and reversed
	{
		IntrusiveList<IntrusiveItem> list;
		IntrusiveItem a(1);
		IntrusiveItem b(2);
		IntrusiveItem c(3);
		list.InsertTail(a);
		list.InsertTail(b);
		list.InsertTail(c);
		list.Remove(b);
		AssertEqual(list.Count(), (size_t)2);
		Assert(!b.IsLinked());
		AssertEqual(list.First().Value, 1);

		int forward[] = { 1, 3 };
		int i = 0;
		for (const IntrusiveItem& item : list)
			AssertEqual(item.Value, forward[i++]);
		AssertEqual(i, 2);

		int reversed[] = { 3, 1 };
		i = 0;
		for (IntrusiveItem& item : list.Reversed())
			AssertEqual(item.Value, reversed[i++]);
		AssertEqual(i, 2);

		list.Remove(a);
		list.Remove(c);
		Assert(list.IsEmpty());
	}

	// A copy of a listed element starts out on no list
	{
		IntrusiveList<IntrusiveItem> list;
		IntrusiveItem a(1);
		list.InsertTail(a);
		const IntrusiveItem copy = a;
		Assert(a.IsLinked());
		Assert(!copy.IsLinked());
		list.Remove(a);
	}
}
