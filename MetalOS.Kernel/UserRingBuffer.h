#pragma once

#include <string>
#include <vector>
#include "MetalOS.Kernel.h"

class UserRingBuffer
{
public:
	UserRingBuffer(const std::string& name, std::vector<paddr_t>& addresses) :
		m_name(name),
		m_addresses(addresses)
	{

	}

	const std::vector<paddr_t>& GetAddresses()
	{
		return m_addresses;
	}

private:
	std::string m_name;
	std::vector<paddr_t> m_addresses;
};
