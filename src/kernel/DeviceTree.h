#pragma once

#include "kernel/KDevice.h"

class DeviceTree
{
public:
	constexpr DeviceTree() : m_root()
	{

	}

	void Enumerate();
	void AddRootChild(KDevice& device);
	void AttachDrivers();
	void Display() const;
	KDevice* GetDeviceByClass(const KDeviceClass kClass) const;

private:
	template <typename Visit>
	KDevice* Walk(Visit visit) const;

	static void AttachDriver(KDevice& device);

	KDevice* m_root;
};
