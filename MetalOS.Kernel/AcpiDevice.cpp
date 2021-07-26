#include "msvc.h"
#include <cstdint>
#include <windows/types.h>
#include "AcpiDevice.h"
extern "C"
{
#include <aclocal.h>
const AH_DEVICE_ID* AcpiAhMatchHardwareId(char* HardwareId);
}
#include "Kernel.h"
#include "Assert.h"

AcpiDevice::AcpiDevice(const ACPI_HANDLE object) :
	Device(),
	m_acpiObject(object),
	m_resources()
{

}

void AcpiDevice::Initialize()
{
	ACPI_BUFFER result = { ACPI_ALLOCATE_BUFFER, nullptr };
	ACPI_STATUS status = AcpiGetName(m_acpiObject, ACPI_FULL_PATHNAME, &result);
	Assert(ACPI_SUCCESS(status));

	Assert(result.Pointer);
	Path = (char*)result.Pointer;
	std::replace(Path.begin(), Path.end(), '.', '\\');
	delete result.Pointer;

	ACPI_DEVICE_INFO* info;
	status = AcpiGetObjectInfo(m_acpiObject, &info);
	Assert(ACPI_SUCCESS(status));

	if (info->Valid & ACPI_VALID_HID)
	{
		m_hid = info->HardwareId.String;

		//Get description
		const AH_DEVICE_ID* id = ::AcpiAhMatchHardwareId(info->HardwareId.String);
		if (id != NULL)
			Description = id->Description;
	}
	
	//These comments were helpful during bringup, probably dont need them anymore
	/*
	Print("F: 0x%02x V: 0x%04x T: 0x%08x\n", info->Flags, info->Valid, info->Type);

	if (info->Valid & ACPI_VALID_ADR)
		Print("    ADR: 0x%x\n", info->Address);

	if (info->Valid & ACPI_VALID_HID)
		Print("    HID: %s\n", info->HardwareId.String);

	if (info->Valid & ACPI_VALID_UID)
		Print("    UID: %s\n", info->UniqueId.String);

	if (info->Valid & ACPI_VALID_CID)
		for (size_t i = 0; i < info->CompatibleIdList.Count; i++)
		{
			Print("    CID: %s\n", info->CompatibleIdList.Ids[i].String);
		}
	*/

	//DDN
	result = { ACPI_ALLOCATE_BUFFER, nullptr };
	status = AcpiEvaluateObjectTyped(m_acpiObject, ACPI_STRING("_DDN"), nullptr, &result, ACPI_TYPE_STRING);
	if (ACPI_SUCCESS(status))
	{
		ACPI_OBJECT* object = (ACPI_OBJECT*)result.Pointer;
		Assert(object);
		Name = object->String.Pointer;
		delete result.Pointer;
	}

	//Wire up resources
	AcpiWalkResources(m_acpiObject, ACPI_STRING("_CRS"), AcpiDevice::AttachResource, this);
}

const void* AcpiDevice::GetResource(uint32_t type) const
{
	for (const auto& resource : this->m_resources)
	{
		if (resource.Type == type)
			return &resource;
	}
	return nullptr;
}

void AcpiDevice::DisplayDetails() const
{
	for (const auto& item : m_resources)
		DisplayResource(item);
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
		//Remove if resource already exists
		pDevice->m_resources.remove_if([&](const ACPI_RESOURCE& _Other) { return _Other.Type == Resource->Type; });
		pDevice->m_resources.push_back(*Resource);
		break;
	}

	return AE_OK;
}

ACPI_STATUS AcpiDevice::DisplayResource(const ACPI_RESOURCE& Resource)
{
	int i;
	switch (Resource.Type) {
	case ACPI_RESOURCE_TYPE_IRQ:
		kernel.Printf("    IRQ: dlen=%d trig=%d pol=%d shar=%d intc=%d cnt=%d int=",
			Resource.Data.Irq.DescriptorLength,
			Resource.Data.Irq.Triggering,
			Resource.Data.Irq.Polarity,
			Resource.Data.Irq.Shareable,
			Resource.Data.Irq.WakeCapable,
			Resource.Data.Irq.InterruptCount);
		for (i = 0; i < Resource.Data.Irq.InterruptCount; i++)
			kernel.Printf("%.02X ", Resource.Data.Irq.Interrupts[i]);
		kernel.Printf("\n");
		break;
	case ACPI_RESOURCE_TYPE_IO:
	{
		kernel.Printf("     IO: Decode=0x%x Align=0x%x AddrLen=%d Min=0x%.04X Max=0x%.04X\n",
			Resource.Data.Io.IoDecode,
			Resource.Data.Io.Alignment,
			Resource.Data.Io.AddressLength,
			Resource.Data.Io.Minimum,
			Resource.Data.Io.Maximum);
	}
		break;
	case ACPI_RESOURCE_TYPE_END_TAG:
		kernel.Printf("    END:\n");
		break;
	case ACPI_RESOURCE_TYPE_ADDRESS16:
		kernel.Printf("    A16: 0x%04X-0x%04X, Gran=0x%04X, Offset=0x%04X\n",
			Resource.Data.Address16.Address.Minimum,
			Resource.Data.Address16.Address.Maximum,
			Resource.Data.Address16.Address.Granularity,
			Resource.Data.Address16.Address.TranslationOffset);
		break;
	case ACPI_RESOURCE_TYPE_ADDRESS32:
		kernel.Printf("    A32: 0x%08X-0x%08X, Gran=0x%08X, Offset=0x%08X\n",
			Resource.Data.Address32.Address.Minimum,
			Resource.Data.Address32.Address.Maximum,
			Resource.Data.Address32.Address.Granularity,
			Resource.Data.Address32.Address.TranslationOffset);
		kernel.Printf("         T: %d, PC: %d, Decode=0x%x, Min=0x%02X, Max=0x%02X\n",
			Resource.Data.Address32.ResourceType,
			Resource.Data.Address32.ProducerConsumer,
			Resource.Data.Address32.Decode,
			Resource.Data.Address32.MinAddressFixed,
			Resource.Data.Address32.MaxAddressFixed);
		break;
	case ACPI_RESOURCE_TYPE_ADDRESS64:
		kernel.Printf("    A64: 0x%016X-0x%016X, Gran=0x%016X, Offset=0x%016X\n",
			Resource.Data.Address64.Address.Minimum,
			Resource.Data.Address64.Address.Maximum,
			Resource.Data.Address64.Address.Granularity,
			Resource.Data.Address64.Address.TranslationOffset);
		kernel.Printf("       : T: %d, PC: %d, Decode=0x%x, Min=0x%02X, Max=0x%02X\n",
			Resource.Data.Address64.ResourceType,
			Resource.Data.Address64.ProducerConsumer,
			Resource.Data.Address64.Decode,
			Resource.Data.Address64.MinAddressFixed,
			Resource.Data.Address64.MaxAddressFixed);
		break;
	case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
		kernel.Printf("   EIRQ: PC=%d Trig=%d Pol=%d Share=%d Wake=%d Cnt=%d Int=",
			Resource.Data.ExtendedIrq.ProducerConsumer,
			Resource.Data.ExtendedIrq.Triggering,
			Resource.Data.ExtendedIrq.Polarity,
			Resource.Data.ExtendedIrq.Shareable,
			Resource.Data.ExtendedIrq.WakeCapable,
			Resource.Data.ExtendedIrq.InterruptCount);
		for (i = 0; i < Resource.Data.ExtendedIrq.InterruptCount; i++)
			kernel.Printf("%.02X ", Resource.Data.ExtendedIrq.Interrupts[i]);
		kernel.Printf("\n");
		break;
	case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
		kernel.Printf("    M32: Addr=0x%016x, Len=0x%x, WP=%d\n",
			Resource.Data.FixedMemory32.Address,
			Resource.Data.FixedMemory32.AddressLength,
			Resource.Data.FixedMemory32.WriteProtect);
		break;
	default:
		kernel.Printf("    Unknown: Type=%d\n", Resource.Type);
		break;

	}
	return AE_OK;
}
