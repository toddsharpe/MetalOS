#include "kernel/Shm.h"
#include "kernel/Grant.h"
#include "kernel/UObject.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Objects/KSpinLock.h"
#include "kernel/Heap.h"
#include "Lib/IntrusiveList.h"
#include "Assert.h"

/*
 * Handle grants: capability-passing between processes. A holder of a handle mints a
 * single-use token (KeGrantShare) that travels over the endpoint registry / IPC
 * rings; the recipient redeems it (KeGrantClaim) to obtain its own handle to the
 * same object. A grant carries the object's type + kernel pointer so the mechanism
 * is generic across handle types; only SharedMemory is wired up so far. The grant
 * holds a reference so the object survives the share->claim gap even if the granter
 * releases it first. Unclaimed grants keep the object alive until claimed or their granter exits.
 */
namespace
{
	struct Grant : ListLink
	{
		Grant(const UObjectType type, void* const object, const uint32_t granter) : Token(), Type(type), Object(object), Granter(granter)
		{

		}

		uint64_t Token;
		UObjectType Type;
		void* Object;
		uint32_t Granter; //Minting process id, or KernelGrantOwner
	};

	KSpinLock m_lock;
	IntrusiveList<Grant> m_grants;
	uint64_t m_nextToken = 1;
}

uint64_t KeGrantShare(const UObjectType type, void* const object, const uint32_t granter)
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

	Grant* const grant = KeAlloc<Grant>(AllocType::Kernel, type, object, granter);
	KSpinLockGuard guard(m_lock);
	grant->Token = m_nextToken++;
	m_grants.InsertTail(*grant);
	return grant->Token;
}

//Drops an exited process's unclaimed grants, releasing the reference each held on its object
void KeGrantRevoke(const uint32_t granter)
{
	KSpinLockGuard guard(m_lock);
	while (true)
	{
		Grant* found = nullptr;
		for (Grant& grant : m_grants)
		{
			if (grant.Granter == granter)
			{
				found = &grant;
				break;
			}
		}
		if (!found)
			return;

		const Grant revoked = *found;
		m_grants.Remove(*found);
		KeFree(found, AllocType::Kernel);
		switch (revoked.Type)
		{
			case UObjectType::SharedMemory:
				KeShmUnref(*static_cast<KSharedMemory*>(revoked.Object));
				break;

			default:
				break;
		}
	}
}

bool KeGrantClaim(const uint64_t token, UObjectType& type, void*& object)
{
	KSpinLockGuard guard(m_lock);
	for (Grant& grant : m_grants)
	{
		if (grant.Token != token)
			continue;

		type = grant.Type;
		object = grant.Object;
		m_grants.Remove(grant); //single-use
		KeFree(&grant, AllocType::Kernel);
		return true;
	}
	return false;
}
