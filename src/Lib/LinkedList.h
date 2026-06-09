#pragma once

#include <cstddef>
#include <cstdlib>
#include "Assert.h"
#include "new.h"

template <typename T>
class LinkedList
{
private:
	struct Node
	{
		Node* Next;
		Node* Prev;
		T Value;
	};

public:
	using ItemType = T;

	constexpr LinkedList() :
		m_head(),
		m_tail(),
		m_count()
	{
	}

	size_t Count() const
	{
		return m_count;
	}

	bool IsEmpty() const
	{
		return m_count == 0;
	}

	//Append to the tail
	bool Add(const T& value)
	{
		Node* node = MakeNode(value);
		if (!node)
			return false;

		node->Prev = m_tail;
		node->Next = nullptr;
		if (m_tail)
			m_tail->Next = node;
		else
			m_head = node;
		m_tail = node;
		m_count++;
		return true;
	}

	//Insert at the head
	bool InsertHead(const T& value)
	{
		Node* node = MakeNode(value);
		if (!node)
			return false;

		node->Next = m_head;
		node->Prev = nullptr;
		if (m_head)
			m_head->Prev = node;
		else
			m_tail = node;
		m_head = node;
		m_count++;
		return true;
	}

	bool Remove(const T& value)
	{
		for (Node* cur = m_head; cur != nullptr; cur = cur->Next)
		{
			if (cur->Value == value)
			{
				Unlink(cur);
				cur->~Node();
				free(cur);
				m_count--;
				return true;
			}
		}
		return false;
	}

	bool Contains(const T& value) const
	{
		for (const Node* n = m_head; n != nullptr; n = n->Next)
		{
			if (n->Value == value)
				return true;
		}
		return false;
	}

	T First() const
	{
		return m_head ? m_head->Value : T{};
	}

	T Last() const
	{
		return m_tail ? m_tail->Value : T{};
	}

	struct Iterator
	{
		Node* Current;

		Iterator(Node* n) :
			Current(n)
		{
		}

		bool operator!=(const Iterator& other) const
		{
			return Current != other.Current;
		}

		Iterator& operator++()
		{
			Current = Current->Next;
			return *this;
		}

		T& operator*()
		{
			return Current->Value;
		}
	};

	struct ConstIterator
	{
		const Node* Current;

		ConstIterator(const Node* n) :
			Current(n)
		{
		}

		bool operator!=(const ConstIterator& other) const
		{
			return Current != other.Current;
		}

		ConstIterator& operator++()
		{
			Current = Current->Next;
			return *this;
		}

		const T& operator*() const
		{
			return Current->Value;
		}
	};

	struct ReverseIterator
	{
		Node* Current;

		ReverseIterator(Node* n) :
			Current(n)
		{
		}

		bool operator!=(const ReverseIterator& other) const
		{
			return Current != other.Current;
		}

		ReverseIterator& operator++()
		{
			Current = Current->Prev;
			return *this;
		}

		T& operator*()
		{
			return Current->Value;
		}
	};

	struct ConstReverseIterator
	{
		const Node* Current;

		ConstReverseIterator(const Node* n) :
			Current(n)
		{
		}

		bool operator!=(const ConstReverseIterator& other) const
		{
			return Current != other.Current;
		}

		ConstReverseIterator& operator++()
		{
			Current = Current->Prev;
			return *this;
		}

		const T& operator*() const
		{
			return Current->Value;
		}
	};

	Iterator begin()
	{
		return {m_head};
	}

	Iterator end()
	{
		return {nullptr};
	}

	ConstIterator begin() const
	{
		return {m_head};
	}

	ConstIterator end() const
	{
		return {nullptr};
	}

	//Proxy enabling range-for in tail -> head order: for (T v : list.Reversed())
	struct ReverseView
	{
		LinkedList* List;
		ReverseIterator begin() const { return {List->m_tail}; }
		ReverseIterator end() const { return {nullptr}; }
	};

	struct ConstReverseView
	{
		const LinkedList* List;
		ConstReverseIterator begin() const { return {List->m_tail}; }
		ConstReverseIterator end() const { return {nullptr}; }
	};

	ReverseView Reversed()
	{
		return {this};
	}

	ConstReverseView Reversed() const
	{
		return {this};
	}

private:
	static Node* MakeNode(const T& value)
	{
		void* mem = malloc(sizeof(Node));
		if (!mem)
			return nullptr;

		Node* node = new (mem)Node();
		node->Value = value;
		return node;
	}

	void Unlink(Node* node)
	{
		if (node->Prev)
			node->Prev->Next = node->Next;
		else
			m_head = node->Next;

		if (node->Next)
			node->Next->Prev = node->Prev;
		else
			m_tail = node->Prev;
	}

	Node* m_head;
	Node* m_tail;
	size_t m_count;
};
