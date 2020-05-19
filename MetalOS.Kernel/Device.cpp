#include "Device.h"
#include "Main.h"

Device::Device() :
	Path(),
	Name(),
	Description(),
	m_hid(),
	m_children(),
	m_driver()
{

}

void Device::Display() const
{
	Print("%s\n", this->Path.c_str());
	Print("    Name: %s\n", this->Name.c_str());
	Print("    Desc: %s\n", this->Description.c_str());
	Print("    HID : %s\n", this->GetHid().c_str());

	this->DisplayDetails();

	for (const auto& child : m_children)
		child->Display();
}
