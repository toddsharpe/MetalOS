#pragma once

#include "KObject.h"

class KFile : public KObject
{
public:
	KFile() :
		KObject(KObjectType::File, false),
		Context(),
		Position(),
		Length(),
		Access()
	{};

	void* Context;
	size_t Position;
	size_t Length;
	GenericAccess Access;
};

