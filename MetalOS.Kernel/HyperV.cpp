#include "HyperV.h"

#include "Cpuid.h"
#include "Main.h"
#include <intrin.h>

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

HyperV::HyperV()
{
	//Verify we are in a hypervisor
	Cpuid cpu;
	Assert(cpu.IsHypervisor());

	int registers[4] = { 0 };

	//Vendor
	__cpuid(registers, HV_CPUID::VENDOR);
	m_highestLeaf = registers[EAX];

	char vendor[13] = { 0 };
	*((uint32_t*)vendor) = (uint32_t)registers[EBX];
	*((uint32_t*)(vendor + sizeof(uint32_t))) = (uint32_t)registers[ECX];
	*((uint32_t*)(vendor + sizeof(uint32_t) * 2)) = (uint32_t)registers[EDX];
	m_vendor = vendor;
	if (m_vendor == "Microsoft Hv")
		m_isPresent = true;

	//Feature identification
	__cpuid(registers, HV_CPUID::FEATURES);
	m_featuresEdx = registers[EDX];
}

void HyperV::ReportGuestID()
{
	GUEST_OS_ID_REG guestId = { 0 };
	guestId.BuildNumber = 0x02;
	guestId.Version = 0x01;
	guestId.OSID = 0x1;
	guestId.OSType = OSType::Linux;//Specify linux for now?
	guestId.OpenSource = true;

	__writemsr(HV_REG::HV_X64_MSR_GUEST_OS_ID, guestId.Value);
}
