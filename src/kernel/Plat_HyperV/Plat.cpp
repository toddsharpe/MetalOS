#include "kernel/Plat.h"
#include "kernel/Plat_HyperV/HyperV.h"
#include "kernel/Plat_HyperV/Drivers/VmBusDriver.h"
#include "kernel/Plat_HyperV/Drivers/HyperVKeyboardDriver.h"
#include "kernel/Plat_HyperV/Drivers/HyperVMouseDriver.h"
#include "kernel/Plat_HyperV/Drivers/HyperVNic.h"
#include "kernel/Plat_HyperV/Drivers/HyperVVideoDriver.h"
#include "Assert.h"
#include "kernel/Devices/KDevice.h"
#include "kernel/Heap.h"

void PlatInitialize()
{
	HyperV::Platform::Initialize();
	HyperV::Interrupts::Initialize();
	HyperV::Tsc::Initialize();
}

void PlatStartTimer(const nano_t period, const uint8_t vector)
{
	HyperV::Timer::Enable(period, 0, vector);
}

void PlatEndOfInterrupt()
{
	HyperV::Interrupts::EOI();
}

nano100_t PlatReadTime()
{
	return HyperV::Tsc::ReadTsc();
}

namespace
{
	template <typename T>
	Driver* Create(KDevice& device)
	{
		Driver* const driver = KeAlloc<T>(AllocType::Kernel, device);
		Assert(driver);
		return driver;
	}
}

Driver* PlatCreateDriver(KDevice& device)
{
	const cstr_t hid = Str(device.Hid);
	if (hid == Str("VMBUS") || hid == Str("MSFT1000"))
		return Create<VmBusDriver>(device);
	if (hid == Str("{F912AD6D-2B17-48EA-BD65-F927A61C7684}"))
		return Create<HyperVKeyboardDriver>(device);
	if (hid == Str("{CFA8B69E-5B4A-4CC0-B98B-8BA1A1F3F95A}"))
		return Create<HyperVMouseDriver>(device);
	if (hid == Str("{F8615163-DF3E-46C5-913F-F2D2F965ED0E}"))
		return Create<HyperVNic>(device);
	if (hid == Str("{DA0A7802-E377-4AAC-8E77-0558EB1073F8}"))
		return Create<HyperVVideoDriver>(device);
	return nullptr;
}

void PlatDisplayDevice(const KDevice& device)
{
	if (device.Type != KDeviceType::Platform)
		return;

	const VmBusChild& child = VmBusChild::Of(device);
	Printf("     - RelId: %d\n", child.RelId);
	Printf("     - MsgConnId: %d\n", child.ConnectionId);
}
