#include "kernel/Kernel_shm.h"
#include "kernel/Kernel_grant.h"
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
 * releases it first. Unclaimed grants keep the object alive until claimed or their granter exits.
 */
namespace
{
	struct Grant
	{
		uint64_t Token;
		UObjectType Type;
		void* Object;
		uint32_t Granter; //Minting process id, or KernelGrantOwner

		bool operator==(const Grant& other) const { return Token == other.Token; }
	};

	KSpinLock m_lock;
	LinkedList<Grant> m_grants;
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

	KSpinLockGuard guard(m_lock);
	const uint64_t token = m_nextToken++;
	Assert(m_grants.Add({ token, type, object, granter }));
	return token;
}

//Drops an exited process's unclaimed grants, releasing the reference each held on its object
void KeGrantRevoke(const uint32_t granter)
{
	KSpinLockGuard guard(m_lock);
	while (true)
	{
		const Grant* found = nullptr;
		for (const Grant& grant : m_grants)
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
		m_grants.Remove(revoked);
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
	for (const Grant& grant : m_grants)
	{
		if (grant.Token != token)
			continue;

		type = grant.Type;
		object = grant.Object;
		m_grants.Remove(grant); //single-use
		return true;
	}
	return false;
}
