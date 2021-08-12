#include "msvc.h"
#include <map>
#include <stack>
#include <crt_stdio.h>
#include "Kernel.h"
#include "Assert.h"
#include "Cpuid.h"
#include "HyperV.h"
#include "VmBusDriver.h"
#include "HyperVKeyboardDriver.h"
#include "HyperVMouseDriver.h"
#include "HyperVScsiDriver.h"
#include "RamDriveDriver.h"
#include "IoApicDriver.h"

#include "DeviceTree.h"

DeviceTree::DeviceTree() :
	m_root()
{

}

void DeviceTree::Populate()
{
	//Discover
	Assert(ACPI_SUCCESS(PopulateAcpi()));
}

void DeviceTree::EnumerateChildren()
{
	//Attach drivers breadth first so enumerated children will get a driver
	std::queue<Device*> queue;
	queue.push(m_root);
	while (!queue.empty())
	{
		Device* current = queue.front();
		queue.pop();

		//TODO: this is gross
		if (current->GetDriver() == nullptr)
		{
			AttachDriver(*current);
			Driver* driver = current->GetDriver();
			if (driver != nullptr)
			{
				driver->Initialize();
			}
		}
		
		{
			Driver* driver = current->GetDriver();
			if (driver != nullptr)
				driver->EnumerateChildren();
		}

		for (auto& child : current->GetChildren())
			queue.push(child);
	}
}

void DeviceTree::AddRootDevice(Device& device)
{
	//Update path - TODO: make Device::AddChild that does this?
	char buffer[64] = { 0 };
	sprintf(buffer, "%s%s", m_root->Path.c_str(), device.GetHid().c_str());
	device.Path = buffer;
	
	m_root->GetChildren().push_back(&device);
}

//Rely on ACPI to discover root
ACPI_STATUS DeviceTree::PopulateAcpi()
{
	//Map of discovered devices
	std::map<ACPI_HANDLE, AcpiDevice*> devices;
	
	//Construct ACPI root device
	ACPI_HANDLE root;
	ACPI_STATUS status = AcpiGetHandle(NULL, ACPI_STRING(ACPI_NS_ROOT_PATH), &root);
	Assert(ACPI_SUCCESS(status));

	AcpiDevice* rootDevice = new AcpiDevice(root);
	rootDevice->Initialize();
	this->m_root = rootDevice;

	devices.insert({ root, rootDevice });
	
	//Walk namespace
	status = AcpiGetDevices(NULL, DeviceTree::AddAcpiDevice, &devices, NULL);
	if (ACPI_FAILURE(status))
	{
		kernel.Printf("Could not AcpiGetDevices: %s(%d)\n", AcpiFormatException(status), status);
		Assert(false);
	}

	return AE_OK;
}

ACPI_STATUS DeviceTree::AddAcpiDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue)
{
	std::map<ACPI_HANDLE, AcpiDevice*>* pDevices = (std::map<ACPI_HANDLE, AcpiDevice*>*)Context;

	//Construct device object
	AcpiDevice* device = new AcpiDevice(Object);
	device->Initialize();

	//Add driver
	AttachDriver(*device);
	Driver* driver = device->GetDriver();
	if (driver != nullptr)
	{
		driver->Initialize();
	}

	//Add new device to map
	pDevices->insert({ Object, device });

	//Add to parent's children if available
	ACPI_HANDLE parent;
	ACPI_STATUS status = AcpiGetParent(Object, &parent);
	if (status == AE_NULL_ENTRY)
		return AE_OK;
	else if (!ACPI_SUCCESS(status))
		return status;

	const auto it = pDevices->find(parent);
	if (it == pDevices->end())
		return AE_NOT_FOUND;

	it->second->GetChildren().push_back(device);
	return AE_OK;
}

void DeviceTree::AttachDriver(Device& device)
{
	//Attach driver
	//TODO: proper pnp/driver manager?
	if (device.GetHid() == "PNP0501")
		device.SetDriver(new UartDriver(device));
	else if (device.GetHid() == "VMBUS")
		device.SetDriver(new VmBusDriver(device));
	else if (device.GetHid() == "{F912AD6D-2B17-48EA-BD65-F927A61C7684}")
		device.SetDriver(new HyperVKeyboardDriver(device));
	//else if (device.GetHid() == "{BA6163D9-04A1-4D29-B605-72E2FFB1DC7F}")
		//device.SetDriver(new HyperVScsiDriver(device));
	else if (device.GetHid() == RamDriveHid)
		device.SetDriver(new RamDriveDriver(device));
	else if (device.GetHid() == "PNP0003")
		device.SetDriver(new IoApicDriver(device));
	//else if (device.GetHid() == "{CFA8B69E-5B4A-4CC0-B98B-8BA1A1F3F95A}")
		//device.SetDriver(new HyperVMouseDriver(device));
}

void DeviceTree::Display() const
{
	Assert(this->m_root);
	this->m_root->Display();
}

Device* DeviceTree::GetDeviceByHid(const std::string& hid) const
{
	std::stack<Device*> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		Device* current = stack.top();
		stack.pop();

		if (current->GetHid() == hid)
			return current;

		for (auto& child : current->GetChildren())
			stack.push(child);
	}

	return nullptr;
}

Device* DeviceTree::GetDeviceByName(const std::string& name) const
{
	std::stack<Device*> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		Device* current = stack.top();
		stack.pop();

		if (current->Name == name)
			return current;

		for (auto& child : current->GetChildren())
			stack.push(child);
	}

	return nullptr;
}

Device* DeviceTree::GetDeviceByType(const DeviceType type) const
{
	std::stack<Device*> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		Device* current = stack.top();
		stack.pop();

		if (current->Type == type)
			return current;

		for (auto& child : current->GetChildren())
			stack.push(child);
	}

	return nullptr;
}

//This could be smarter and split the path to search. For now just DFS
Device* DeviceTree::GetDevice(const std::string& path) const
{
	std::stack<Device*> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		Device* current = stack.top();
		stack.pop();

		if (current->Path == path)
			return current;

		for (auto& child : current->GetChildren())
			stack.push(child);
	}

	return nullptr;
}

