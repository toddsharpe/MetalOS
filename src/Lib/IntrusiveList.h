#pragma once

#include <cstddef>
#include "Assert.h"

template <typename T>
class IntrusiveList;

//What an element derives from to be on an IntrusiveList; an element is on at most one list
class ListLink
{
public:
	constexpr ListLink() : m_next(), m_prev()
	{

	}

	//A copied element starts out on no list, and assigning one leaves the target where it is
	constexpr ListLink(const ListLink&) : m_next(), m_prev()
	{

	}

	ListLink& operator=(const ListLink&)
	{
		return *this;
	}

	bool IsLinked() const
	{
		return m_next != nullptr;
	}

private:
	template <typename T>
	friend class IntrusiveList;

	ListLink* m_next;
	ListLink* m_prev;
};

//Doubly linked list of elements that derive from ListLink: inserting never allocates and removing is O(1); zero-initialized is empty
template <typename T>
class IntrusiveList
{
	using Link = ListLink;

public:
	constexpr IntrusiveList() : m_head(), m_count()
	{

	}

	IntrusiveList(const IntrusiveList&) = delete;
	IntrusiveList& operator=(const IntrusiveList&) = delete;

	size_t Count() const
	{
		return m_count;
	}

	bool IsEmpty() const
	{
		return m_count == 0;
	}

	void InsertHead(T& item)
	{
		Link& head = Head();
		Splice(item, head, *head.m_next);
	}

	void InsertTail(T& item)
	{
		Link& head = Head();
		Splice(item, *head.m_prev, head);
	}

	void Remove(T& item)
	{
		Link& link = item;
		Assert(link.IsLinked() && link.m_next->m_prev == &link && link.m_prev->m_next == &link);
		link.m_prev->m_next = link.m_next;
		link.m_next->m_prev = link.m_prev;
		link.m_next = nullptr;
		link.m_prev = nullptr;
		m_count--;
	}

	T& First()
	{
		Assert(!IsEmpty());
		return Element(*m_head.m_next);
	}

	const T& First() const
	{
		Assert(!IsEmpty());
		return Element(*m_head.m_next);
	}

	T& Pop()
	{
		T& first = First();
		Remove(first);
		return first;
	}

	template <typename Value, typename Node, bool Forward>
	class Iterator
	{
	public:
		explicit Iterator(Node* const node) : m_node(node)
		{

		}

		bool operator!=(const Iterator& other) const
		{
			return m_node != other.m_node;
		}

		Iterator& operator++()
		{
			m_node = Forward ? m_node->m_next : m_node->m_prev;
			return *this;
		}

		Value& operator*() const
		{
			return static_cast<Value&>(*m_node);
		}

	private:
		Node* m_node;
	};

	template <typename Value, typename Node>
	class ReverseRange
	{
	public:
		explicit ReverseRange(Node& head) : m_head(head)
		{

		}

		Iterator<Value, Node, false> begin() const
		{
			return Iterator<Value, Node, false>(m_head.m_prev ? m_head.m_prev : &m_head);
		}

		Iterator<Value, Node, false> end() const
		{
			return Iterator<Value, Node, false>(&m_head);
		}

	private:
		Node& m_head;
	};

	Iterator<T, Link, true> begin()
	{
		return Iterator<T, Link, true>(m_head.m_next ? m_head.m_next : &m_head);
	}

	Iterator<T, Link, true> end()
	{
		return Iterator<T, Link, true>(&m_head);
	}

	Iterator<const T, const Link, true> begin() const
	{
		return Iterator<const T, const Link, true>(m_head.m_next ? m_head.m_next : &m_head);
	}

	Iterator<const T, const Link, true> end() const
	{
		return Iterator<const T, const Link, true>(&m_head);
	}

	//Tail to head
	ReverseRange<T, Link> Reversed()
	{
		return ReverseRange<T, Link>(m_head);
	}

	ReverseRange<const T, const Link> Reversed() const
	{
		return ReverseRange<const T, const Link>(m_head);
	}

private:
	static T& Element(Link& link)
	{
		return static_cast<T&>(link);
	}

	static const T& Element(const Link& link)
	{
		return static_cast<const T&>(link);
	}

	//The head links to itself once anything has been on the list
	Link& Head()
	{
		if (!m_head.m_next)
		{
			m_head.m_next = &m_head;
			m_head.m_prev = &m_head;
		}
		return m_head;
	}

	void Splice(T& item, Link& prev, Link& next)
	{
		Link& link = item;
		Assert(!link.IsLinked());
		Assert(prev.m_next == &next && next.m_prev == &prev);
		link.m_prev = &prev;
		link.m_next = &next;
		prev.m_next = &link;
		next.m_prev = &link;
		m_count++;
	}

	Link m_head;
	size_t m_count;
};
