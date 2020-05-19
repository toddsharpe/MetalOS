#pragma once
#include "Driver.h"

class IoApicDriver : public Driver
{
public:
	IoApicDriver(Device& device);

	Result Initialize() override;
	Result Read(const char* buffer, size_t length) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;
};

