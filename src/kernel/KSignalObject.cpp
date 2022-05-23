#include "KSignalObject.h"

KSignalObject::KSignalObject(const KObjectType type) : 
	KObject(type)
{

}

bool KSignalObject::IsSyncObj() const
{
	return true;
}
