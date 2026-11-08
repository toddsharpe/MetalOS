#include "kernel/Shm.h"
#include "kernel/Input.h"
#include "kernel/Core/KProcess.h"
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
namespace
{
	static constexpr uint32_t Capacity = 512;
	static constexpr uint32_t MoveLimit = Capacity / 2; //mouse moves stop going in at half full, keeping the rest for keys and clicks

	KSpinLock m_lock;
	void* m_region = nullptr;
	MouseButtonState m_buttons = {}; //buttons of the last mouse event that went in
}

void KeInputInitialize()
{
	const size_t size = SharedRing<Message>::RegionSize(Capacity);
	m_region = KePublishRegion(Wm::InputEndpoint, size, [](void* const region) { SharedRing<Message>::Init(region, Capacity); });
}

void KeInputPost(const Message& message)
{
	if (!m_region)
		return;

	KSpinLockGuard guard(m_lock);
	SharedRing<Message> ring(m_region, Capacity); //WM can rewrite the header; use our capacity

	//A move only repositions the cursor and the next one carries the new position, so moves give way when the WM falls behind
	const bool mouse = message.Header.MessageType == MessageType::MouseEvent;
	if (mouse && message.MouseEvent.Buttons == m_buttons && ring.Count() >= MoveLimit)
		return;

	if (ring.Enqueue(message) && mouse)
		m_buttons = message.MouseEvent.Buttons;
}
