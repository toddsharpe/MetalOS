#pragma once
#include "KObject.h"

class KSignalObject : public KObject
{
public:
	KSignalObject(const KObjectType type);

	virtual bool IsSignalled() const = 0;

	virtual bool IsSyncObj() const override;

};

