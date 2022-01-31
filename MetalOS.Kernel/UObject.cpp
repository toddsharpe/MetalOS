#include "UObject.h"

UObject::UObject(KObject& object, const UserObjectType type) :
	m_object(object),
	m_type(type)
{
	m_object.IncRefCount();
}

UObject::~UObject()
{
	m_object.DecRefCount();
}

UserObjectType UObject::GetType() const
{
	return m_type;
}

KObject& UObject::GetObject() const
{
	return m_object;
}

bool UObject::IsPipe() const
{
	return m_type == UserObjectType::ReadPipe || m_type == UserObjectType::WritePipe;
}

bool UObject::IsEvent() const
{
	return m_type == UserObjectType::Event;
}

bool UObject::IsWriteable() const
{
	return m_type == UserObjectType::File || m_type == UserObjectType::WritePipe || m_type == UserObjectType::Debug;
}

bool UObject::IsReadable() const
{
	return m_type == UserObjectType::File || m_type == UserObjectType::ReadPipe;
}
