#include "AcpiDeviceTree.h"
#include "Main.h"

#include "msvc.h"
#include <map>
#include <stack>

AcpiDeviceTree::AcpiDeviceTree()
{

}

ACPI_STATUS AcpiDeviceTree::Populate()
{
	//Map of discovered devices
	std::map<ACPI_HANDLE, AcpiDevice*> devices;
	
	//Construct root device
	ACPI_HANDLE root;
	ACPI_STATUS status = AcpiGetHandle(NULL, ACPI_STRING(ACPI_NS_ROOT_PATH), &root);
	Assert(ACPI_SUCCESS(status));
	AcpiDevice* rootDevice = new AcpiDevice(root);
	rootDevice->Initialize();
	this->m_root = rootDevice;

	devices.insert({ root, rootDevice });
	
	//Walk namespace
	status = AcpiGetDevices(NULL, AcpiDeviceTree::AddAcpiDevice, &devices, NULL);
	if (ACPI_FAILURE(status))
	{
		Print("Could not AcpiGetDevices: %s(%d)\n", AcpiFormatException(status), status);
		Assert(false);
	}

	return AE_OK;
}

ACPI_STATUS AcpiDeviceTree::AddAcpiDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue)
{
	std::map<ACPI_HANDLE, AcpiDevice*>* pDevices = (std::map<ACPI_HANDLE, AcpiDevice*>*)Context;

	//Construct device object
	AcpiDevice* device = new AcpiDevice(Object);
	device->Initialize();

	//Attach driver
	//TODO: proper pnp/driver manager?
	if (device->GetHid() == "PNP0501")
		device->SetDriver(new UartDriver(*device));

	//Add new device to map
	pDevices->insert({ Object, device });

	//Add to parents children if available
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

void AcpiDeviceTree::PrintTree() const
{
	std::stack<AcpiDevice*> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		AcpiDevice* current = stack.top();
		stack.pop();

		Print("%s - %s - %s\n", current->GetFullName(), current->GetHid(), current->GetName());
		for (auto& child : current->GetChildren())
			stack.push(child);
	}
}

bool AcpiDeviceTree::GetDeviceByHid(const std::string& hid, AcpiDevice** device)
{
	std::stack<AcpiDevice*> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		AcpiDevice* current = stack.top();
		stack.pop();

		if (current->GetHid() == hid)
		{
			*device = current;
			return true;
		}

		for (auto& child : current->GetChildren())
			stack.push(child);
	}

	return false;
}
bool AcpiDeviceTree::GetDeviceByName(const std::string& name, AcpiDevice** device)
{
	std::stack<AcpiDevice*> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		AcpiDevice* current = stack.top();
		stack.pop();

		if (current->GetName() == name)
		{
			*device = current;
			return true;
		}

		for (auto& child : current->GetChildren())
			stack.push(child);
	}

	return false;
}
