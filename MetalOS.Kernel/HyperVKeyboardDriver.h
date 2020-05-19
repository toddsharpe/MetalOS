#pragma once

#include "Device.h"
#include "Driver.h"

class HyperVKeyboardDriver : public Driver
{
public:
	HyperVKeyboardDriver(Device& device);

	Result Initialize() override;
	Result Read(const char* buffer, size_t length) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

private:

};

