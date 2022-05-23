#pragma once
#include "KSpinLock.h"

enum class KObjectType
{
	Thread,
	Process,
	Semaphore,
	Predicate,
	File,
	Pipe,
	Event
};

class KObject
{
public:
	KObject(const KObjectType type);

	void IncRefCount();
	void DecRefCount();

	bool IsClosed() const;

	KObjectType GetType() const
	{
		return m_type;
	}

	virtual bool IsSyncObj() const;
	virtual void Dispose();

private:
	size_t m_refCount;
	KSpinLock m_spinLock;
	KObjectType m_type;
};

