#include "HyperV.h"

#include "Cpuid.h"
#include "Main.h"
#include <intrin.h>

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

HyperV::HV_REFERENCE_TSC_PAGE HyperV::TscPage = { 0 };

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
	m_featuresEax = registers[EAX];
	m_featuresEbx = registers[EBX];
	m_featuresEdx = registers[EDX];
}

void HyperV::Initialize()
{
	Print("HyperV::Initialize\n");

	Assert(this->IsPresent());
	Assert(this->DirectSyntheticTimers());
	Assert(this->AccessPartitionReferenceCounter());
	
	GUEST_OS_ID_REG guestId = { 0 };
	guestId.BuildNumber = 0x02;
	guestId.Version = 0x01;
	guestId.OSID = 0x1;
	guestId.OSType = OSType::Linux;//Specify linux for now?
	guestId.OpenSource = true;
	__writemsr(HV_REG::HV_X64_MSR_GUEST_OS_ID, guestId.Value);

	//Enable TSC
	HV_REF_TSC_REG tscReg = { 0 };
	tscReg.AsUint64 = __readmsr(HV_REG::HV_X64_MSR_REFERENCE_TSC);
	tscReg.Enable = true;
	tscReg.GPAPageNumber = kernel.VirtualToPhysical((uint64_t)&TscPage) >> PAGE_SHIFT;
	__writemsr(HV_REG::HV_X64_MSR_REFERENCE_TSC, tscReg.AsUint64);

	Print("  Tsc: 0x%016x\n", ReadTsc());
}

//HyperV TLFS 6.0 12.7.3.3
//Returns value in 100ns increments
nano100_t HyperV::ReadTsc()
{
	uint64_t Scale, Offset;
	uint32_t StartSequence, EndSequence;
	uint64_t Tsc;

	do {
		StartSequence = TscPage.TscSequence;
		Assert(StartSequence);

		Tsc = __rdtsc();
		// Assigning Scale and Offset should neither happen before
		// setting StartSequence, nor after setting EndSequence.
		Scale = TscPage.TscScale;
		Offset = TscPage.TscOffset;
		EndSequence = TscPage.TscSequence;
	} while (EndSequence != StartSequence);
	// The result of the multiplication is treated as a 128-bit value.
	int64_t highProduct = 0;
	int64_t lowProduct = _mul128(Tsc, Scale, &highProduct);
	return highProduct + Offset;
}
