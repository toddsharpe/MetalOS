#include "AcpiDevice.h"
#include "Main.h"

AcpiDevice::AcpiDevice(ACPI_HANDLE object) :
	m_acpiObject(object),
	m_fullName(),
	m_name(),
	m_children()
{

}

ACPI_STATUS AcpiDevice::Initialize()
{
	ACPI_BUFFER result = { ACPI_ALLOCATE_BUFFER, nullptr };
	ACPI_STATUS status = AcpiGetName(m_acpiObject, ACPI_FULL_PATHNAME, &result);
	if (!ACPI_SUCCESS(status))
		return status;

	Assert(result.Pointer);
	m_fullName = (char*)result.Pointer;
	delete result.Pointer;

	ACPI_DEVICE_INFO* info;
	status = AcpiGetObjectInfo(m_acpiObject, &info);
	if (!ACPI_SUCCESS(status))
		return status;

	if (info->Valid & ACPI_VALID_HID)
		m_hid = info->HardwareId.String;
	
	//DDN
	result = { ACPI_ALLOCATE_BUFFER, nullptr };
	status = AcpiEvaluateObjectTyped(m_acpiObject, ACPI_STRING("_DDN"), nullptr, &result, ACPI_TYPE_STRING);
	if (ACPI_SUCCESS(status))
	{
		ACPI_OBJECT* object = (ACPI_OBJECT*)result.Pointer;
		Assert(object);
		m_name = object->String.Pointer;
		delete result.Pointer;
	}

	//Wire up resources
	AcpiWalkResources(m_acpiObject, ACPI_STRING("_CRS"), AcpiDevice::AttachResource, this);

	return status;
}

//TODO: this should convert ACPI descriptors to abstract ones, but for now just bin them
ACPI_STATUS AcpiDevice::AttachResource(ACPI_RESOURCE* Resource, void* Context)
{
	AcpiDevice* pDevice = (AcpiDevice*)Context;
	
	switch (Resource->Type)
	{
		case ACPI_RESOURCE_TYPE_ADDRESS32:
		case ACPI_RESOURCE_TYPE_ADDRESS64:
		case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
		case ACPI_RESOURCE_TYPE_IRQ:
		case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
		case ACPI_RESOURCE_TYPE_IO:
			pDevice->m_resources.push_back(*Resource);
			break;
	}

	return AE_OK;
}

const ACPI_RESOURCE* AcpiDevice::GetResource(uint32_t type) const
{
	for (const auto& resource : this->m_resources)
	{
		if (resource.Type == type)
			return &resource;
	}
	return nullptr;
}
