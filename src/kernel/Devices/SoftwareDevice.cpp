#include "SoftwareDevice.h"
#include "Assert.h"

SoftwareDevice::SoftwareDevice(const std::string& hid, void* context) :
	Device(),
	m_context(context)
{
	this->m_hid = hid;
}

void SoftwareDevice::Initialize()
{

}

const void* SoftwareDevice::GetResource(uint32_t type) const
{
	const ResourceType t = (ResourceType)type;
	switch (t)
	{
	case ResourceType::Context:
		return m_context;
		break;
	}
	return nullptr;
}

void SoftwareDevice::DisplayDetails() const
{
	Printf("    Context: 0x%016x\n", m_context);
}
