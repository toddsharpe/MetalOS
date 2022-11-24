#include "KEvent.h"

KEvent::KEvent(bool manualReset, bool initialState) :
	KSignalObject(KObjectType::Event),
	m_manualReset(manualReset),
	m_state(initialState)
{

}

void KEvent::Set()
{
	m_state = true;
}

void KEvent::Reset()
{
	m_state = false;
}

bool KEvent::IsManual()
{
	return m_manualReset;
}

bool KEvent::IsSignalled() const
{
	return m_state;
}