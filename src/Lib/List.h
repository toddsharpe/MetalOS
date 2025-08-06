#pragma once

#include "core_crt/stdint.h"
#include "Assert.h"

// Implementation of Windows double-linked lists (see list.h)

#define LIST_CONTAINING_RECORD(address, type, field) ((type *)((uintptr_t)(address) - \
															   (uintptr_t)(&((type *)0)->field)))
struct ListEntry
{
	ListEntry *Flink;
	ListEntry *Blink;
};

struct ListHead
{
	ListEntry Link;
	size_t Count;
};

inline void ListInitializeHead(ListEntry *listHead)
{
	listHead->Flink = listHead->Blink = listHead;
}

inline void ListInitializeHead(ListHead& head)
{
	ListInitializeHead(&head.Link);
	head.Count = 0;
}

inline bool ListIsEmpty(const ListEntry *listHead)
{
	return (listHead->Flink == listHead);
}

inline bool ListIsEmpty(const ListHead& listHead)
{
	return ListIsEmpty(&listHead.Link);
}

inline void ListRemoveEntry(ListEntry *entry)
{
	ListEntry *flink = entry->Flink;
	ListEntry *blink = entry->Blink;
	blink->Flink = flink;
	flink->Blink = blink;
	
	//Unlink element
	entry->Blink = nullptr;
	entry->Flink = nullptr;
}

inline void ListRemoveEntry(ListHead& head, ListEntry &entry)
{
	Assert(head.Count > 0);
	head.Count--;
	ListRemoveEntry(&entry);
}

inline ListEntry *ListRemoveHead(ListEntry *listHead)
{
	ListEntry *flink = listHead->Flink;
	ListRemoveEntry(flink);
	return flink;
}

inline ListEntry *ListRemoveHead(ListHead& head)
{
	return ListRemoveHead(&head.Link);
}

inline void ListInsertHead(ListEntry *listHead, ListEntry *entry)
{
	ListEntry *flink = listHead->Flink;
	entry->Flink = flink;
	entry->Blink = listHead;
	flink->Blink = entry;
	listHead->Flink = entry;
}

inline void ListInsertHead(ListHead& head, ListEntry &entry)
{
	head.Count++;
	ListInsertHead(&head.Link, &entry);
}

inline void ListInsertTail(ListEntry *listHead, ListEntry *entry)
{
	ListEntry *blink = listHead->Blink;
	entry->Flink = listHead;
	entry->Blink = blink;
	blink->Flink = entry;
	listHead->Blink = entry;
}

inline void ListInsertTail(ListHead& head, ListEntry &entry)
{
	head.Count++;
	ListInsertTail(&head.Link, &entry);
}



/*
inline bool ListAny(const ListEntry *head, const ListPredicate pred)
{
	ListEntry* current = head->Flink;
	bool result = false;
	while (current != head)
	{
		result |= pred(current);
		entry = entry->Flink;
	}
	return result;
}
*/

template <typename TItem, typename TContext>
using ListFuncCtx = void(*)(const ListEntry&, TItem&, TContext const);
template <typename T>
using ListFunc = void(*)(const ListEntry&, T&);
template <typename T>
using ListCFunc = void(*)(const ListEntry&, const T&);
template <typename TItem, typename TContext>
using ListPredCtx = bool(*)(const ListEntry&, const TItem&, TContext&);
template <typename T>
using ListPred = bool(*)(const ListEntry&, T&);

template <typename TItem, typename TContext>
inline void ListForEach(const ListHead& head, const ListFuncCtx<TItem, TContext>& func, TContext ctx)
{
	if (!head.Count)
		return;
	
	for (ListEntry* link = head.Link.Flink; link != &head.Link; link = link->Flink)
	{
		TItem* item = LIST_CONTAINING_RECORD(link, TItem, Link);
		func(*link, *item, ctx);
	}
}

template <typename T>
inline void ListForEach(const ListHead &head, const ListFunc<T>& func)
{
	if (!head.Count)
		return;
	
	for (ListEntry* link = head.Link.Flink; link != &head.Link; link = link->Flink)
	{
		T* item = LIST_CONTAINING_RECORD(link, T, Link);
		func(*link, *item);
	}
}

template <typename T>
inline void ListForEach(const ListHead &head, const ListCFunc<T>& func)
{
	if (!head.Count)
		return;
	
	for (ListEntry* link = head.Link.Flink; link != &head.Link; link = link->Flink)
	{
		T* item = LIST_CONTAINING_RECORD(link, T, Link);
		func(*link, *item);
	}
}

template <typename TItem, typename TContext>
inline TItem* ListFirst(const ListHead& head, const ListPredCtx<TItem, TContext>& pred, TContext& ctx)
{
	if (!head.Count)
		return nullptr;
	
	for (ListEntry* link = head.Link.Flink; link != &head.Link; link = link->Flink)
	{
		TItem* item = LIST_CONTAINING_RECORD(link, TItem, Link);
		if (pred(*link, *item, ctx))
			return item;
	}
	return nullptr;
}

template <typename TItem, typename TContext>
inline bool ListAny(const ListHead& head, const ListPredCtx<TItem, TContext>& pred, TContext& ctx)
{
	if (!head.Count)
		return false;
	
	return ListFirst(head, pred, ctx) != nullptr;
}

template <typename T>
inline T* ListGet(const ListHead& head, const size_t index)
{
	if (!head.Count)
		return nullptr;
	
	int i = 0;
	for (ListEntry* link = head.Link.Flink; link != &head.Link; link = link->Flink)
	{
		T* item = LIST_CONTAINING_RECORD(link, T, Link);
		if (i == index)
		{
			return item;
		}

		i++;
	}

	return nullptr;
}

template <typename T>
struct ListHead2
{
	template <typename TContext>
	using CAction = void(*)(const T&, TContext&);

	template <typename TContext>
	using Pred = bool(*)(const T&, TContext&);
	
	ListHead Head;

	void Initialize()
	{
		ListInitializeHead(Head);
	}

	size_t Count() const
	{
		return Head.Count;
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

	void Display() const
	{
		Printf("Count: %d\n", Count());
		for (ListEntry* link = Head.Link.Flink; link != &Head.Link; link = link->Flink)
		{
			T* item = LIST_CONTAINING_RECORD(link, T, Link);
			Printf("Item: 0x%016x\n", item);
		}
	}
};
