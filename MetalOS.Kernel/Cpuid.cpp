#include "Cpuid.h"

#include <intrin.h>
#include <cstdint>

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

// On Intel 64 processors, CPUID clears the high 32 bits of the RAX/RBX/RCX/RDX registers in all modes.
//https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=vs-2019
//https://en.wikipedia.org/wiki/CPUID
Cpuid::Cpuid()
{
	int registers[4] = { 0 };

	//CPUID 0
	__cpuid(registers, 0);

	m_highestFunction = (uint32_t)registers[EAX];

	char vendor[13] = { 0 };
	*((uint32_t*)vendor) = (uint32_t)registers[EBX];
	*((uint32_t*)(vendor + sizeof(uint32_t))) = (uint32_t)registers[EDX];
	*((uint32_t*)(vendor + sizeof(uint32_t) * 2)) = (uint32_t)registers[ECX];
	m_vendor = vendor;
	if (m_vendor == "GenuineIntel")
		m_isIntel = true;

	//CPUID 1
	__cpuid(registers, 1);
	m_func1Ecx = (uint32_t)registers[ECX];
	m_func1Edx = (uint32_t)registers[EDX];

	//CPUID Extended Functions
	__cpuid(registers, 0x80000000);
	m_highestExtendedFunction = (uint32_t)registers[EAX];
}
