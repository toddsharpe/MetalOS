#include "Driver.h"

Driver::Driver(AcpiDevice& device) : m_device(device)
{

}

void Driver::Write(const std::string& str)
{
	this->Write(str.c_str(), str.length());
}
