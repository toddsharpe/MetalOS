#include "Cpuid.h"

#include <intrin.h>
#include <cstdint>


// On Intel 64 processors, CPUID clears the high 32 bits of the RAX/RBX/RCX/RDX registers in all modes.
//https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=vs-2019
//https://en.wikipedia.org/wiki/CPUID
Cpuid::Cpuid()
{
	int registers[4] = { 0 };

	//CPUID 0
	__cpuid(registers, 0);
	uint32_t highestFunction = (uint32_t)registers[Regs::EAX];

	char vendor[13] = { 0 };
	*((uint32_t*)vendor) = (uint32_t)registers[Regs::EBX];
	*((uint32_t*)(vendor + sizeof(uint32_t))) = (uint32_t)registers[Regs::EDX];
	*((uint32_t*)(vendor + sizeof(uint32_t) * 2)) = (uint32_t)registers[Regs::ECX];
	m_vendor = vendor;
	if (m_vendor == "GenuineIntel")
		m_isIntel = true;
	else if (m_vendor == "AuthenticAMD")
		m_isAmd = true;

	//CPUID 1
	__cpuid(registers, 1);
	m_func1Ecx = (uint32_t)registers[ECX];
}
