#pragma once

#include "kernel/Devices/KDevice.h"

class DeviceTree
{
public:
	constexpr DeviceTree() : m_root()
	{

	}

	void Enumerate();
	void AttachDrivers();
	void Display() const;
	KDevice* GetDeviceByClass(const KDeviceClass kClass) const;

private:
	template <typename Visit>
	KDevice* Walk(Visit visit) const;

	static void AttachDriver(KDevice& device);

	KDevice* m_root;
};
