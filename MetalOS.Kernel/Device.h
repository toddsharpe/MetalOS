#pragma once

#include "msvc.h"
#include <string>
#include <list>
#include "Driver.h"

class Driver;
class Device
{
public:
	Device();

	virtual void Initialize() = 0;
	virtual const void* GetResource(uint32_t type) const = 0;
	virtual void DisplayDetails() const = 0;

	const std::string& GetHid() const
	{
		return m_hid;
	}

	std::list<Device*>& GetChildren()
	{
		return this->m_children;
	}

	void SetDriver(Driver* driver)
	{
		m_driver = driver;
	}

	Driver* GetDriver()
	{
		return m_driver;
	}

	void Display() const;

	std::string Name;
	std::string Path;
	std::string Description;

protected:
	std::string m_hid;
	std::list<Device*> m_children;

	Driver* m_driver;
};

