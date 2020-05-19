#include "HyperVDevice.h"
#include <crt_stdio.h>
#include "Main.h"

HyperVDevice::HyperVDevice(vmbus_channel_offer_channel& channel) :
	Device(),
	m_channel(channel)
{

}

void HyperVDevice::Initialize()
{
	char buffer[64] = { 0 };
	crt_sprintf(buffer, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		m_channel.offer.if_type.Data1,
		m_channel.offer.if_type.Data2,
		m_channel.offer.if_type.Data3,
		m_channel.offer.if_type.Data4[0], m_channel.offer.if_type.Data4[1], m_channel.offer.if_type.Data4[2], m_channel.offer.if_type.Data4[3],
		m_channel.offer.if_type.Data4[4], m_channel.offer.if_type.Data4[5], m_channel.offer.if_type.Data4[6], m_channel.offer.if_type.Data4[7]);

	m_hid = buffer;

	GetDeviceName(m_hid, Name);
	Description = Name;
}

const void* HyperVDevice::GetResource(uint32_t type) const
{
	return nullptr;
}

void HyperVDevice::DisplayDetails() const
{

}

const bool HyperVDevice::GetDeviceName(const std::string& hid, std::string& name)
{
	const VmBusDevice* current;
	for (current = VMBUS_DEVICE_NAMES; current->Name; current++)
	{
		if (hid == current->Hid)
		{
			name = current->Name;
			return true;
		}
	}

	return false;
}
