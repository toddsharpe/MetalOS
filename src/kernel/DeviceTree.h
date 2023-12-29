#pragma once

#include "Kernel/Devices/Device.h"
#include "Driver.h"
#include "Kernel/Devices/AcpiDevice.h"

class DeviceTree
{
public:
	DeviceTree();

	void Populate();
	void EnumerateChildren();
	void AddRootDevice(Device& device);
	
	void Display() const;

	Device* GetDeviceByHid(const std::string& hid) const;
	Device* GetDeviceByName(const std::string& name) const;
	Device* GetDeviceByType(const DeviceType type) const;
	Device* GetDevice(const std::string& path) const;

private:
	static ACPI_STATUS AddAcpiDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue);
	static void AttachDriver(Device& device);

	ACPI_STATUS PopulateAcpi();

	Device* m_root;
};

