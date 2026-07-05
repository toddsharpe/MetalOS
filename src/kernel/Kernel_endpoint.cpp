#include "kernel/Api.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/StaticVector.h"
#include "Lib/StaticQueue.h"
#include "Assert.h"
#include <cstring>

//Global name-based IPC rendezvous. Two uses, both mediated by the kernel so
//unrelated processes never race on shared memory directly:
//  - Register/Lookup: a name -> shared-memory-id map for a server's well-known
//    single region (e.g. the "input" ring).
//  - Post/Poll: a per-endpoint queue of client-supplied ids (connection handoff).
//    A client Posts the id of a channel it created; the server Polls to accept it.
//Everything above this (channel layout, message rings) is arranged inside shared
//memory by usermode convention; the kernel only moves ids across, under a lock.
namespace _EP
{
	struct Endpoint
	{
		char Name[32];
		uint64_t Id;
		StaticQueue<uint64_t, 32> Pending;
	};

	KSpinLock m_lock;
	StaticVector<Endpoint, 16> m_endpoints;

	//Caller must hold m_lock
	Endpoint* Find(const char* const name)
	{
		for (Endpoint& ep : m_endpoints)
		{
			if (strcmp(ep.Name, name) == 0)
				return &ep;
		}
		return nullptr;
	}

	//Caller must hold m_lock
	Endpoint* FindOrCreate(const char* const name)
	{
		Endpoint* ep = Find(name);
		if (ep)
			return ep;

		Endpoint created = {};
		strncpy(created.Name, name, sizeof(created.Name) - 1);
		if (!m_endpoints.Add(created))
			return nullptr;
		return Find(name);
	}
}

bool KeEndpointRegister(const char* const name, const uint64_t id)
{
	KSpinLockGuard guard(_EP::m_lock);
	_EP::Endpoint* ep = _EP::FindOrCreate(name);
	if (!ep)
		return false;
	ep->Id = id;
	return true;
}

bool KeEndpointLookup(const char* const name, uint64_t& id)
{
	KSpinLockGuard guard(_EP::m_lock);
	_EP::Endpoint* ep = _EP::Find(name);
	if (!ep)
		return false;
	id = ep->Id;
	return true;
}

bool KeEndpointPost(const char* const name, const uint64_t value)
{
	KSpinLockGuard guard(_EP::m_lock);
	_EP::Endpoint* ep = _EP::FindOrCreate(name);
	if (!ep)
		return false;
	return ep->Pending.Enqueue(value);
}

bool KeEndpointPoll(const char* const name, uint64_t& value)
{
	KSpinLockGuard guard(_EP::m_lock);
	_EP::Endpoint* ep = _EP::Find(name);
	if (!ep || ep->Pending.IsEmpty())
		return false;
	value = ep->Pending.Dequeue();
	return true;
}
