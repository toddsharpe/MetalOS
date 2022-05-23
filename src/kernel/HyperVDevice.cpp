#include "Kernel.h"
#include "Assert.h"

#include "HyperVDevice.h"

HyperVDevice::HyperVDevice(vmbus_channel_offer_channel& channel, uint32_t conn_id) :
	Device(),
	m_channel(channel),
	m_msg_conn_id(conn_id)
{

}

void HyperVDevice::Initialize()
{
	char buffer[64] = { 0 };
	sprintf(buffer, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
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
	ResourceType t = (ResourceType)type;
	switch (t)
	{
	case ResourceType::ChildRelid:
		return &m_channel.child_relid;
		break;
	case ResourceType::ConnectionId:
		return &m_msg_conn_id;
		break;

	case ResourceType::OfferChannel:
		return &this->m_channel;
		break;
	}
	return nullptr;
}

void HyperVDevice::DisplayDetails() const
{
	kernel.Printf("    RelId: %d\n", m_channel.child_relid);
	kernel.Printf("    MonitorId: %d\n", m_channel.monitorid);
	kernel.Printf("    Monitored: %d\n", m_channel.monitor_allocated);
	kernel.Printf("    Connection: %d\n", m_channel.connection_id);
	kernel.Printf("    IsDedicated: %d\n", m_channel.is_dedicated_interrupt);
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
