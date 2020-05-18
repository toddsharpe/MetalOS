#include "KObject.h"

KObject::KObject() : m_refCount(1), m_spinLock()
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

bool KObject::IsClosed()
{
	return m_refCount == 0;
}
