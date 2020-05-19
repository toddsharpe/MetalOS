#include "HyperVKeyboardDriver.h"

HyperVKeyboardDriver::HyperVKeyboardDriver(Device& device) :
	Driver(device)
{

}

Result HyperVKeyboardDriver::Initialize()
{
	return Result::ResultNotImplemented;
}

Result HyperVKeyboardDriver::Read(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result HyperVKeyboardDriver::Write(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result HyperVKeyboardDriver::EnumerateChildren()
{
	return Result::ResultNotImplemented;
}
