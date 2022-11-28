#include "KObject.h"

KObject::KObject(const KObjectType type, const bool isSyncObj) : Type(type), IsSyncObj(isSyncObj), m_refCount(1)
{

}

void KObject::IncRefCount()
{
	cpu_flags_t flags = m_spinLock.Acquire();
	m_refCount++;
	m_spinLock.Release(flags);
}

void KObject::DecRefCount()
{
	cpu_flags_t flags = m_spinLock.Acquire();
	m_refCount--;
	m_spinLock.Release(flags);
}

bool KObject::IsClosed() const
{
	return m_refCount == 0;
}

void KObject::Dispose()
{
	//Empty
}
