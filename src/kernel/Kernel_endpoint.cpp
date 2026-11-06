#include "kernel/Api.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/StaticVector.h"
#include "Lib/CircularBuffer.h"
#include "Assert.h"
#include <cstring>

//Global name-based IPC rendezvous: Register/Lookup publish an opaque value (e.g. a grant token) under a name, Post/Poll queue client values for connection handoff
namespace
{
	struct Endpoint
	{
		char Name[MaxEndpointName];
		bool Registered; //False while it only holds Posts made before its server registered
		uint32_t Owner; //Registering process id, or KernelEndpointOwner
		uint64_t Id;
		CircularBuffer<uint64_t, 32> Pending;
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
	KSpinLockGuard guard(m_lock);
	Endpoint* ep = FindOrCreate(name);
	if (!ep || (ep->Registered && ep->Owner != owner))
		return false;
	ep->Registered = true;
	ep->Owner = owner;
	ep->Id = id;
	return true;
}

bool KeEndpointLookup(const char* const name, uint64_t& id)
{
	KSpinLockGuard guard(m_lock);
	Endpoint* ep = Find(name);
	if (!ep || !ep->Registered)
		return false;
	id = ep->Id;
	return true;
}

bool KeEndpointPost(const char* const name, const uint64_t value)
{
	KSpinLockGuard guard(m_lock);
	Endpoint* ep = FindOrCreate(name);
	if (!ep)
		return false;
	return ep->Pending.Enqueue(value);
}

bool KeEndpointPoll(const char* const name, const uint32_t owner, uint64_t& value)
{
	KSpinLockGuard guard(m_lock);
	Endpoint* ep = Find(name);
	if (!ep || !ep->Registered || ep->Owner != owner || ep->Pending.IsEmpty())
		return false;
	value = ep->Pending.Dequeue();
	return true;
}

//Drops an exited process's endpoints so a restarted server can register them again
void KeEndpointRelease(const uint32_t owner)
{
	KSpinLockGuard guard(m_lock);
	size_t i = 0;
	while (i < m_endpoints.Count())
	{
		const Endpoint& ep = m_endpoints.begin()[i];
		if (ep.Registered && ep.Owner == owner)
			m_endpoints.RemoveAt(i);
		else
			i++;
	}
}
