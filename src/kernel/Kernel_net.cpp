#include "kernel/Api.h"
#include "kernel/KProcess.h"
#include "kernel/DeviceTree.h"
#include "kernel/Drivers/Driver.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/SharedRing.h"
#include "user/Protocol_net.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).

namespace
{
	KSpinLock m_lock;
	void* m_rxRegion = nullptr;

	NicDriver* Nic()
	{
		KDevice* const device = m_deviceTree.GetDeviceByClass(KDeviceClass::Nic);
		return device ? static_cast<NicDriver*>(device->Driver) : nullptr;
	}
}

void KeInitNetDevice()
{
	using namespace NetDevice;

	//The RX ring of raw frames, for netstack to claim
	const size_t size = SharedRing<Frame>::RegionSize(RxCapacity);
	m_rxRegion = KePublishRegion(RxEndpoint, size, [](void* const region) { SharedRing<Frame>::Init(region, RxCapacity); });
}

void KeNetRxFrame(NicDriver* const nic, const void* const frame, const size_t length)
{
	if (!m_rxRegion || length == 0 || length > NetDevice::Mtu)
		return;

	KSpinLockGuard guard(m_lock);
	SharedRing<NetDevice::Frame> ring(m_rxRegion, NetDevice::RxCapacity); //netstack can rewrite the header; use our capacity

	static constexpr uint32_t ifIdx = 0; //only one NIC for now

	NetDevice::Frame record = {};
	record.IfIdx = ifIdx;
	record.Length = static_cast<uint16_t>(length);
	memcpy(record.Data, frame, length);
	ring.Enqueue(record); //drops silently if netstack has fallen behind
}

bool KeNetSend(const uint32_t ifIdx, const void* const frame, const size_t length)
{
	if (!frame || length == 0 || length > NetDevice::Mtu)
		return false;

	NicDriver* const nic = KeNetGetInterface(ifIdx);
	return nic && nic->SendFrame(frame, length);
}

size_t KeNetInterfaceCount()
{
	return Nic() ? 1 : 0;
}

NicDriver* KeNetGetInterface(const size_t index)
{
	//Only one interface; index comes from usermode (NetSend)
	if (index != 0)
		return nullptr;

	//Always returns the only interface
	NicDriver* const nic = Nic();
	return nic;
}
