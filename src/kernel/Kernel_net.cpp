#include "kernel/Api.h"
#include "kernel/KProcess.h"
#include "kernel/UObject.h"
#include "kernel/DeviceTree.h"
#include "kernel/Drivers/Driver.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/SharedRing.h"
#include "NetDevice.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;
extern DeviceTree m_deviceTree;

namespace _NETDEV
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

	const size_t size = SharedRing<Frame>::RegionSize(RxCapacity);
	KSharedMemory* shm = KeShmCreate(size);
	Assert(shm);

	//The kernel holds this ring for the life of the system.
	shm->Refs.Increment();
	_NETDEV::m_rxRegion = KeShmMap(m_process, *shm);
	Assert(_NETDEV::m_rxRegion);
	SharedRing<Frame>::Init(_NETDEV::m_rxRegion, RxCapacity);

	//Publish the RX ring (as a capability grant) for netstack to claim.
	Assert(KeEndpointRegister(RxEndpoint, KeGrantShare(UObjectType::SharedMemory, shm)));
}

void KeNetRxFrame(NicDriver* const nic, const void* const frame, const size_t length)
{
	if (!_NETDEV::m_rxRegion || length == 0 || length > NetDevice::Mtu)
		return;

	KSpinLockGuard guard(_NETDEV::m_lock);
	SharedRing<NetDevice::Frame> ring(_NETDEV::m_rxRegion);

	const uint32_t ifIdx = 0; //only one NIC for now

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
	return nic && nic->SendFrame(frame, length) == Result::Success;
}

size_t KeNetInterfaceCount()
{
	return _NETDEV::Nic() ? 1 : 0;
}

NicDriver* KeNetGetInterface(const size_t index)
{
	Assert(index == 0);

	//Always returns the only interface
	NicDriver* const nic = _NETDEV::Nic();
	return nic;
}
