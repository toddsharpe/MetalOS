#include "Kernel.h"

#include "Device.h"

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
	kernel.Printf("%s\n", this->Path.c_str());
	kernel.Printf("    Name: %s\n", this->Name.c_str());
	kernel.Printf("    Desc: %s\n", this->Description.c_str());
	kernel.Printf("    HID : %s\n", this->GetHid().c_str());

	this->DisplayDetails();

	for (const auto& child : m_children)
		child->Display();
}
