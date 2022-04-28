#include "KObject.h"

KObject::KObject(const KObjectType type) : m_refCount(1), m_spinLock(), m_type(type)
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

bool KObject::IsSyncObj() const
{
	return false;
}

void KObject::Dispose()
{

}
