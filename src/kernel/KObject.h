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
	Event,
	Sleep
};

class KObject
{
public:
	KObject(const KObjectType type, const bool isSyncObj);

	void IncRefCount();
	void DecRefCount();
	bool IsClosed() const;

	virtual void Dispose();

	const KObjectType Type;
	const bool IsSyncObj;

private:
	size_t m_refCount;
	KSpinLock m_spinLock; //TODO(tsharpe): remove?
};

