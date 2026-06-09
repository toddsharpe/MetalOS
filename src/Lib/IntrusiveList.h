#pragma once

#include "Lib/List.h"

template <typename T>
struct IntrusiveList
{
	template <typename TContext>
	using CAction = void(*)(const T&, TContext&);

	template <typename TContext>
	using Pred = bool(*)(const T&, TContext&);

	constexpr IntrusiveList() :
		Head()
	{

	}

	void Initialize()
	{
		ListInitializeHead(Head);
	}

	size_t Count() const
	{
		return Head.Count;
	}

	bool IsEmpty() const
	{
		return Count() == 0;
	}

	template <typename TContext>
	void ForEach(const CAction<TContext> action, TContext ctx) const
	{
		if (Count() == 0)
			return;

		for (ListEntry* link = Head.Link.Flink; link != &Head.Link; link = link->Flink)
		{
			T* item = LIST_CONTAINING_RECORD(link, T, Link);
			action(*item, ctx);
		}
	}

	template <typename TContext>
	void ForEachReverse(const CAction<TContext> action, TContext ctx) const
	{
		if (Count() == 0)
			return;

		for (ListEntry* link = Head.Link.Blink; link != &Head.Link; link = link->Blink)
		{
			T* item = LIST_CONTAINING_RECORD(link, T, Link);
			action(*item, ctx);
		}
	}

	template <typename TContext>
	T* First(const Pred<TContext> pred, TContext ctx) const
	{
		if (!Head.Count)
			return nullptr;
	
		for (ListEntry* link = Head.Link.Flink; link != &Head.Link; link = link->Flink)
		{
			T* item = LIST_CONTAINING_RECORD(link, T, Link);
			if (pred(*item, ctx))
				return item;
		}
		return nullptr;
	}

	void InsertHead(T& item)
	{
		ListInsertHead(Head, item.Link);
	}

	void InsertTail(T& item)
	{
		ListInsertTail(Head, item.Link);
	}

	void Remove(T& item)
	{
		ListRemoveEntry(Head, item.Link);
	}

	T* Pop()
	{
		Assert(!IsEmpty());
		ListEntry* popped = ListRemoveHead(Head);
		return LIST_CONTAINING_RECORD(popped, T, Link);
	}

	void Display() const
	{
		Printf("Count: %d\n", Count());
		for (ListEntry* link = Head.Link.Flink; link != &Head.Link; link = link->Flink)
		{
			T* item = LIST_CONTAINING_RECORD(link, T, Link);
			Printf("Item: 0x%016x\n", item);
		}
	}

	ListHead Head;
};
