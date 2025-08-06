#include "Lib/StaticStack.h"

template <typename TItem, size_t NMax, size_t NCount>
void CheckArray(StaticStack<TItem, NMax>& stack, TItem (&&expected)[NCount])
{
	size_t index = 0;
	for (const auto& i : stack)
	{
		Assert(i == expected[index]);
		index++;
	}
}

void StaticStack_test()
{
	StaticStack<int, 4> stack;
	stack.Push(1);
	stack.Push(2);
	stack.Push(3);
	CheckArray(stack, {3, 2, 1});

	stack.Evict(2);
	CheckArray(stack, {3, 1});

	stack.Push(4);
	CheckArray(stack, {4, 3, 1});

	stack.Evict(4);
	CheckArray(stack, {3, 1});

	stack.Evict(3);
	CheckArray(stack, {1});
}
