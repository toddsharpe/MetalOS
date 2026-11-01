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
		char Name[MaxEndpointName];
		bool Registered; //False while it only holds Posts made before its server registered
		uint32_t Owner; //Registering process id, or KernelEndpointOwner
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

		//A truncated name would collide with another and never match itself in Find
		if (strlen(name) >= MaxEndpointName)
			return nullptr;

		Endpoint created = {};
		strcpy(created.Name, name);
		if (!m_endpoints.Add(created))
			return nullptr;
		return Find(name);
	}
}

//The first registrant owns the endpoint until it exits; only it can re-register or poll it
bool KeEndpointRegister(const char* const name, const uint64_t id, const uint32_t owner)
{
	KSpinLockGuard guard(_EP::m_lock);
	_EP::Endpoint* ep = _EP::FindOrCreate(name);
	if (!ep || (ep->Registered && ep->Owner != owner))
		return false;
	ep->Registered = true;
	ep->Owner = owner;
	ep->Id = id;
	return true;
}

bool KeEndpointLookup(const char* const name, uint64_t& id)
{
	KSpinLockGuard guard(_EP::m_lock);
	_EP::Endpoint* ep = _EP::Find(name);
	if (!ep || !ep->Registered)
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

bool KeEndpointPoll(const char* const name, const uint32_t owner, uint64_t& value)
{
	KSpinLockGuard guard(_EP::m_lock);
	_EP::Endpoint* ep = _EP::Find(name);
	if (!ep || !ep->Registered || ep->Owner != owner || ep->Pending.IsEmpty())
		return false;
	value = ep->Pending.Dequeue();
	return true;
}

//Drops an exited process's endpoints so a restarted server can register them again
void KeEndpointRelease(const uint32_t owner)
{
	KSpinLockGuard guard(_EP::m_lock);
	size_t i = 0;
	while (i < _EP::m_endpoints.Count())
	{
		const _EP::Endpoint& ep = _EP::m_endpoints.begin()[i];
		if (ep.Registered && ep.Owner == owner)
			_EP::m_endpoints.RemoveAt(i);
		else
			i++;
	}
}
