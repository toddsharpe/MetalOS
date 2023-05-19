#pragma once

#include <cstdint>

// Implementation of Windows double-linked lists (see list.h)

struct ListEntry
{
	ListEntry *Flink;
	ListEntry *Blink;
};

inline void ListInitializeHead(ListEntry *listHead)
{
	listHead->Flink = listHead->Blink = listHead;
}

inline bool ListIsEmpty(const ListEntry *listHead)
{
	return (listHead->Flink == listHead);
}

inline void ListRemoveEntry(ListEntry *entry)
{
	ListEntry *flink = entry->Flink;
	ListEntry *blink = entry->Blink;
	blink->Flink = flink;
	flink->Blink = blink;
}

inline ListEntry *ListRemoveHead(ListEntry *listHead)
{
	ListEntry *flink = listHead->Flink;
	ListRemoveEntry(flink);
	return flink;
}

inline void ListInsertHead(ListEntry *listHead, ListEntry *entry)
{
	ListEntry *flink = listHead->Flink;
	entry->Flink = flink;
	entry->Blink = listHead;
	flink->Blink = entry;
	listHead->Flink = entry;
}

inline void ListInsertTail(ListEntry *listHead, ListEntry *entry)
{
	ListEntry *blink = listHead->Blink;
	entry->Flink = listHead;
	entry->Blink = blink;
	blink->Flink = entry;
	listHead->Blink = entry;
}

#define LIST_CONTAINING_RECORD(address, type, field) ((type *)((uintptr_t)(address) - \
															   (uintptr_t)(&((type *)0)->field)))
