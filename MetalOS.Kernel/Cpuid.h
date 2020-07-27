#pragma once
#include "msvc.h"
#include <string>
#include <bitset>

class Cpuid
{
public:
	Cpuid();

	bool IsIntel() const
	{
		return m_isIntel;
	}

	const std::string& GetVendor() const
	{
		return m_vendor;
	}

	bool IsHypervisor()
	{
		return m_func1Ecx[31];
	}

private:
	typedef std::bitset<std::numeric_limits<uint32_t>::digits> bitset_32;

	std::string m_vendor;
	bool m_isIntel;

	bitset_32 m_func1Ecx;
};

