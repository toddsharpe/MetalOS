#pragma once

//Consider: https://github.com/calum74/linq

namespace Linq
{
	template <typename TItem, typename TContext>
	using ListPredCtx = bool(*)(const TItem&, TContext);

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
}
