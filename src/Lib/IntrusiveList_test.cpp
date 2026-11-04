#include "Lib/IntrusiveList.h"

struct IntrusiveItem
{
	int Value;
	ListEntry Link;
};

void IntrusiveList_test()
{
	// Pop drops the count, so the list reports empty once drained
	{
		IntrusiveList<IntrusiveItem> list;
		list.Initialize();
		IntrusiveItem a = { 1 };
		IntrusiveItem b = { 2 };
		list.InsertHead(a);
		list.InsertHead(b);
		AssertEqual(list.Count(), (size_t)2);

		AssertEqual(list.Pop()->Value, 2);
		AssertEqual(list.Count(), (size_t)1);
		AssertEqual(list.Pop()->Value, 1);
		AssertEqual(list.Count(), (size_t)0);
		Assert(list.IsEmpty());
	}

	// Remove and InsertTail keep the count in step
	{
		IntrusiveList<IntrusiveItem> list;
		list.Initialize();
		IntrusiveItem a = { 1 };
		IntrusiveItem b = { 2 };
		IntrusiveItem c = { 3 };
		list.InsertTail(a);
		list.InsertTail(b);
		list.InsertTail(c);
		list.Remove(b);
		AssertEqual(list.Count(), (size_t)2);

		int expected[] = { 1, 3 };
		int i = 0;
		for (const IntrusiveItem& item : list)
			AssertEqual(item.Value, expected[i++]);
		AssertEqual(i, 2);

		list.Pop();
		list.Pop();
		Assert(list.IsEmpty());
	}
}
