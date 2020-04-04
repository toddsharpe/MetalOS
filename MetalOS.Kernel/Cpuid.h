#pragma once
#include "msvc.h"
#include <string>
#include <bitset>

class Cpuid
{
public:
	Cpuid();

	bool IsIntel() { return m_isIntel;  }
	
	bool HighestFunction() { return m_highestFunction; }
	bool HighestExtendedFunction() { return m_highestExtendedFunction; }

	bool HyperThreading() { return m_func1Edx[28]; }
	bool Apic() { return m_func1Edx[9]; }
	bool X2Apic() { return m_func1Ecx[21]; }
	bool Hypervisor() { return m_func1Ecx[31]; }

private:
	typedef std::bitset<std::numeric_limits<uint32_t>::digits> bitset_32;

	std::string m_vendor;
	bool m_isIntel;

	uint32_t m_highestFunction;
	uint32_t m_highestExtendedFunction;

	bitset_32 m_func1Ecx;
	bitset_32 m_func1Edx;

	bitset_32 m_funcE1Ecx;
	bitset_32 m_funcE1Edx;


};

