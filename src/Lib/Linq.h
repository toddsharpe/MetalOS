#pragma once

#include <cstddef>
#include "Assert.h"

//Consider: https://github.com/calum74/linq
//
//Generic, read-only algorithms over any container exposing `using ItemType = T` and range-for.
//Structural/mutating operations (insert, remove, O(1) Count/IsEmpty) stay on the container.

namespace Linq
{
	template <typename TItem, typename TContext>
	using ListPredCtx = bool(*)(const TItem&, TContext);

	template <typename TItem, typename TContext>
	using ListActionCtx = void(*)(const TItem&, TContext);

	template <typename TList>
	inline const typename TList::ItemType* First(const TList& list)
	{
		for (const typename TList::ItemType& item : list)
			return &item;

		return nullptr;
	}

	template <typename TList, typename TContext>
	inline const typename TList::ItemType* First(const TList& list, ListPredCtx<typename TList::ItemType, TContext> func, TContext ctx)
	{
		for (const typename TList::ItemType& item : list)
		{
			if (func(item, ctx))
				return &item;
		}

		return nullptr;
	}

	template <typename TList, typename TContext>
	inline bool Any(const TList& list, ListPredCtx<typename TList::ItemType, TContext> func, TContext ctx)
	{
		for (const typename TList::ItemType& item : list)
		{
			if (func(item, ctx))
				return true;
		}

		return false;
	}

	//Invoke action for each item.
	template <typename TList, typename TContext>
	inline void ForEach(const TList& list, ListActionCtx<typename TList::ItemType, TContext> func, TContext ctx)
	{
		for (const typename TList::ItemType& item : list)
			func(item, ctx);
	}

	//O(n) element count (use the container's own Count() when it stores one).
	template <typename TList>
	inline size_t Count(const TList& list)
	{
		size_t count = 0;
		for ([[maybe_unused]] const typename TList::ItemType& item : list)
			count++;
		return count;
	}

	//Count of elements matching the predicate.
	template <typename TList, typename TContext>
	inline size_t Count(const TList& list, ListPredCtx<typename TList::ItemType, TContext> func, TContext ctx)
	{
		size_t count = 0;
		for (const typename TList::ItemType& item : list)
			if (func(item, ctx))
				count++;
		return count;
	}

	//Debug: print each element's address.
	template <typename TList>
	inline void Dump(const TList& list)
	{
		Printf("Count: %d\n", Count(list));
		for (const typename TList::ItemType& item : list)
			Printf("Item: 0x%016x\n", &item);
	}
}
