#include "kernel/Api.h"
#include "kernel/UObject.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/LinkedList.h"
#include "Assert.h"

/*
 * Handle grants: capability-passing between processes. A holder of a handle mints a
 * single-use token (KeGrantShare) that travels over the endpoint registry / IPC
 * rings; the recipient redeems it (KeGrantClaim) to obtain its own handle to the
 * same object. A grant carries the object's type + kernel pointer so the mechanism
 * is generic across handle types; only SharedMemory is wired up so far. The grant
 * holds a reference so the object survives the share->claim gap even if the granter
 * releases it first. Unclaimed grants keep the object alive until claimed.
 */
namespace _GRANT
{
	struct Grant
	{
		uint64_t Token;
		UObjectType Type;
		void* Object;

		bool operator==(const Grant& other) const { return Token == other.Token; }
	};

	KSpinLock m_lock;
	LinkedList<Grant> m_grants;
	uint64_t m_nextToken = 1;
}

uint64_t KeGrantShare(const UObjectType type, void* const object)
{
	//Take a type-specific reference so the object outlives the pending grant.
	switch (type)
	{
		case UObjectType::SharedMemory:
			static_cast<KSharedMemory*>(object)->Refs.Increment();
			break;

		default:
			break;
	}

	KSpinLockGuard guard(_GRANT::m_lock);
	const uint64_t token = _GRANT::m_nextToken++;
	Assert(_GRANT::m_grants.Add({ token, type, object }));
	return token;
}

bool KeGrantClaim(const uint64_t token, UObjectType& type, void*& object)
{
	KSpinLockGuard guard(_GRANT::m_lock);
	for (const _GRANT::Grant& grant : _GRANT::m_grants)
	{
		if (grant.Token != token)
			continue;

		type = grant.Type;
		object = grant.Object;
		_GRANT::m_grants.Remove(grant); //single-use
		return true;
	}
	return false;
}
