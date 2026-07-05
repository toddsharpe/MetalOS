#include "kernel/Api.h"
#include "kernel/KProcess.h"
#include "kernel/UObject.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/SharedRing.h"
#include "user/MetalOS.UI.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;

//Kernel -> WM input event channel. The mouse/keyboard drivers post Message
//records here (from driver/interrupt context); the usermode WM maps the region
//via the well-known "input" endpoint and drains it. This is an SPSC ring (kernel
//is the single logical producer, WM the single consumer) with the producer side
//serialized by a spinlock because more than one driver posts.
namespace _INPUT
{
	constexpr uint32_t Capacity = 128;
	constexpr const char* EndpointName = "input";

	KSpinLock m_lock;
	void* m_region = nullptr;
}

void KeInputInitialize()
{
	const size_t size = SharedRing<Message>::RegionSize(_INPUT::Capacity);
	KSharedMemory* shm = KeShmCreate(size);
	Assert(shm);

	//The kernel holds this region for the life of the system; take a handle reference
	//for its permanent mapping (mapping itself no longer bumps the refcount).
	shm->Refs.Increment();
	_INPUT::m_region = KeShmMap(m_process, *shm);
	Assert(_INPUT::m_region);
	SharedRing<Message>::Init(_INPUT::m_region, _INPUT::Capacity);

	//Publish a capability token for the WM to claim (KeGrantShare takes its own ref).
	Assert(KeEndpointRegister(_INPUT::EndpointName, KeGrantShare(UObjectType::SharedMemory, shm)));
}

void KeInputPost(const Message& message)
{
	if (!_INPUT::m_region)
		return;

	KSpinLockGuard guard(_INPUT::m_lock);
	SharedRing<Message> ring(_INPUT::m_region);
	ring.Enqueue(message); //drops silently if the WM has fallen behind
}
