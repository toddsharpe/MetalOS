#pragma once

#include "KObject.h"
#include "UserObjectType.h"

class UObject
{
public:
	UObject(KObject& object, const UserObjectType type);
	~UObject();

	UserObjectType GetType() const;
	//TODO: make template
	KObject& GetObject() const;

	template<typename T>
	T& GetObject() const
	{
		return (T&)(m_object);
	}

	bool IsPipe() const;
	bool IsEvent() const;

	bool IsWriteable() const;
	bool IsReadable() const;

private:
	KObject& m_object;
	UserObjectType m_type;
};
