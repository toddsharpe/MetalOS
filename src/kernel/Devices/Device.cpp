#include "Kernel/Devices/Device.h"
#include "Assert.h"

Device::Device() :
	Path(),
	Name(),
	Description(),
	Type(DeviceType::Unknown),
	m_hid(),
	m_children(),
	m_driver()
{

}

void Device::Display() const
{
	Printf("%s\n", this->Path.c_str());
	Printf("    Name: %s\n", this->Name.c_str());
	Printf("    Desc: %s\n", this->Description.c_str());
	Printf("    HID : %s\n", this->GetHid().c_str());

	this->DisplayDetails();

	for (const auto& child : m_children)
		child->Display();
}
