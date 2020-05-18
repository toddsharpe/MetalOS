#pragma once
#include "KSpinLock.h"

class KObject
{
public:
	KObject();

	void IncRefCount();
	void DecRefCount();

	bool IsClosed();

private:
	size_t m_refCount;
	KSpinLock m_spinLock;
};

