#pragma once

#include "Lib/List.h"

//Intrusive doubly-linked list (elements embed a `ListEntry Link`). Holds only the structural
//core: insert, remove, O(1) Count/IsEmpty. Range-for + ItemType make it Linq-compatible, so
//queries (First/Any/ForEach/Count-by-predicate) come from Linq.h and work on every container.
template <typename T>
struct IntrusiveList
{
	using ItemType = T;

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

	//Range-for support (yields T), so Linq algorithms operate on this list.
	struct Iterator
	{
		ListEntry* Node;
		bool operator!=(const Iterator& other) const { return Node != other.Node; }
		Iterator& operator++() { Node = Node->Flink; return *this; }
		T& operator*() const { return *LIST_CONTAINING_RECORD(Node, T, Link); }
	};

	struct ConstIterator
	{
		const ListEntry* Node;
		bool operator!=(const ConstIterator& other) const { return Node != other.Node; }
		ConstIterator& operator++() { Node = Node->Flink; return *this; }
		const T& operator*() const { return *LIST_CONTAINING_RECORD(Node, T, Link); }
	};

	Iterator begin() { return { Head.Link.Flink }; }
	Iterator end() { return { &Head.Link }; }
	ConstIterator begin() const { return { Head.Link.Flink }; }
	ConstIterator end() const { return { &Head.Link }; }

	ListHead Head;
};
