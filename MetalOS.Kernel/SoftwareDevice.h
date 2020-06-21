#pragma once

#include "msvc.h"

#include "Device.h"

class SoftwareDevice : public Device
{
public:
	SoftwareDevice(const std::string& hid, void* context = nullptr);

	void Initialize() override;
	const void* GetResource(uint32_t type) const override;
	void DisplayDetails() const override;

	enum class ResourceType
	{
		Context
	};

private:
	void* m_context;
};

