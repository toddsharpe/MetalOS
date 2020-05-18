#include "VmBusDriver.h"
#include "Main.h"

VmBusDriver::VmBusDriver(AcpiDevice& device) : Driver(device)
{

}

void VmBusDriver::Write(const char* buffer, size_t length)
{
	Assert(false);
}
