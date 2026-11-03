#include "kernel/Api.h"
#include "kernel/KProcess.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/SharedRing.h"
#include "user/MetalOS.UI.h"
#include "user/Protocol_wm.h"
#include "Assert.h"

//Kernel -> WM input event channel. The mouse/keyboard drivers post Message
//records here (from driver/interrupt context); the usermode WM maps the region
//via the well-known "input" endpoint and drains it. This is an SPSC ring (kernel
//is the single logical producer, WM the single consumer) with the producer side
//serialized by a spinlock because more than one driver posts.
namespace _INPUT
{
	constexpr uint32_t Capacity = 128;

	KSpinLock m_lock;
	void* m_region = nullptr;
}

void KeInputInitialize()
{
	const size_t size = SharedRing<Message>::RegionSize(_INPUT::Capacity);
	_INPUT::m_region = KePublishRegion(Wm::InputEndpoint, size, [](void* const region) { SharedRing<Message>::Init(region, _INPUT::Capacity); });
}

void KeInputPost(const Message& message)
{
	if (!_INPUT::m_region)
		return;

	KSpinLockGuard guard(_INPUT::m_lock);
	SharedRing<Message> ring(_INPUT::m_region, _INPUT::Capacity); //WM can rewrite the header; use our capacity
	ring.Enqueue(message); //drops silently if the WM has fallen behind
}
