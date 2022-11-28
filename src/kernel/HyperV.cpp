//TLFS: Hypervisor Top Level Functional Specification v6.0b.pdf

#include "Kernel.h"
#include "Assert.h"

#include "HyperV.h"

#include "Cpuid.h"
#include <intrin.h>

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

volatile HyperV::HV_REFERENCE_TSC_PAGE HyperV::TscPage = { 0 };

volatile HV_MESSAGE HyperV::SynicMessages[HV_SYNIC_SINT_COUNT] = { HvMessageTypeNone, 0 };
volatile HV_SYNIC_EVENT_FLAGS HyperV::SynicEvents[HV_SYNIC_SINT_COUNT] = { 0 };
volatile uint8_t HyperV::HypercallPage[PAGE_SIZE] = { 0 };
volatile uint8_t HyperV::PostMessagePage[PAGE_SIZE] = { 0 };

HyperV::HyperV() :
	m_highestLeaf(),
	m_vendor(),
	m_isPresent()
{
	
}

//TLFS: 3.13
void HyperV::Initialize()
{
	Printf("HyperV::Initialize\n");
	
	//1. Verify we are in a hypervisor
	Cpuid cpu;
	Assert(cpu.IsHypervisor());

	int registers[4] = { 0 };

	//2. Read highest leaf and interface signature
	__cpuid(registers, HV_CPUID::VENDOR);
	m_highestLeaf = registers[EAX];

	//2.1 Vendor name
	char vendor[13] = { 0 };
	*((uint32_t*)vendor) = (uint32_t)registers[EBX];
	*((uint32_t*)(vendor + sizeof(uint32_t))) = (uint32_t)registers[ECX];
	*((uint32_t*)(vendor + sizeof(uint32_t) * 2)) = (uint32_t)registers[EDX];
	m_vendor = vendor;
	if (m_vendor == "Microsoft Hv")
		m_isPresent = true;

	//2.2 Interface
	__cpuid(registers, HV_CPUID::INTERFACE);
	char interface[5] = { 0 };
	*((uint32_t*)interface) = (uint32_t)registers[EAX];
	Assert(strcmp(interface, "Hv#1") == 0);

	//Feature identification
	__cpuid(registers, HV_CPUID::FEATURES);
	m_featuresEax = registers[EAX];
	m_featuresEbx = registers[EBX];
	m_featuresEdx = registers[EDX];

	//Recommendation
	__cpuid(registers, HV_CPUID::PERF_RECOMMENDATIONS);
	m_recommendationsEax = registers[EAX];
	
	//Assert settings
	Assert(this->IsPresent());
	Assert(this->DirectSyntheticTimers());
	Assert(this->AccessPartitionReferenceCounter());
	Assert(!this->DeprecateAutoEOI());
	
	//3. Write identity
	GUEST_OS_ID_REG guestId = { 0 };
	guestId.BuildNumber = 0x02;
	guestId.Version = 0x01;
	guestId.OSID = 0x1;
	guestId.OSType = OSType::Linux;//Specify linux for now?
	guestId.OpenSource = true;
	__writemsr(HV_REG::HV_X64_MSR_GUEST_OS_ID, guestId.Value);

	//4-7. Enable/map hypercall
	{
		HV_HYPERCALL_REGISTER reg = { 0 };
		reg.AsUint64 = __readmsr(HV_REG::HV_X64_MSR_HYPERCALL);
		
		if (!reg.Enabled)
		{
			reg.Enabled = true;
			reg.HypercallGPFN = kernel.VirtualToPhysical((paddr_t)HypercallPage) >> PAGE_SHIFT;
			__writemsr(HV_REG::HV_X64_MSR_HYPERCALL, reg.AsUint64);
		}
	}

	//Initialize Synic
	HyperV::SetSynicEventPage(kernel.VirtualToPhysical((paddr_t)SynicEvents));
	HyperV::SetSynicMessagePage(kernel.VirtualToPhysical((paddr_t)SynicMessages));

	//Enable TSC
	HV_REF_TSC_REG tscReg = { 0 };
	tscReg.AsUint64 = __readmsr(HV_REG::HV_X64_MSR_REFERENCE_TSC);
	tscReg.Enable = true;
	tscReg.GPAPageNumber = kernel.VirtualToPhysical((uintptr_t)& TscPage) >> PAGE_SHIFT;
	__writemsr(HV_REG::HV_X64_MSR_REFERENCE_TSC, tscReg.AsUint64);
	Printf("  Tsc: 0x%016x\n", ReadTsc());
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

typedef uint64_t(*Hypercall)(uint64_t, uint64_t, uint64_t);

HV_HYPERCALL_RESULT_VALUE HyperV::HvPostMessage(
	__in HV_CONNECTION_ID ConnectionId,
	__in HV_MESSAGE_TYPE MessageType,
	__in UINT32 PayloadSize,
	__in_ecount(PayloadSize)
	PCVOID Message
)
{
	Assert(PayloadSize <= HV_MESSAGE_PAYLOAD_BYTE_COUNT);
	
	cpu_flags_t flags = ArchDisableInterrupts();
	
	HV_POST_MESSAGE_INPUT* input = (HV_POST_MESSAGE_INPUT*)PostMessagePage;
	input->ConnectionId = ConnectionId;
	input->MessageType = MessageType;
	input->PayloadSize = PayloadSize;
	memcpy(input->Payload, Message, PayloadSize);

	HV_HYPERCALL_INPUT_VALUE inputValue = { 0 };
	inputValue.CallCode = HypercallCodes::PostMessage;

	Hypercall entry = (Hypercall)(void*)HypercallPage;

	HV_HYPERCALL_RESULT_VALUE status = { 0 };
	status.AsUint64 = entry(inputValue.AsUint64, kernel.VirtualToPhysical((uintptr_t)input), NULL);

	ArchRestoreFlags(flags);
	return status;
}

HV_HYPERCALL_RESULT_VALUE
HyperV::HvSignalEvent(
	__in HV_CONNECTION_ID ConnectionId,
	__in UINT16 FlagNumber //relative to base number for port
)
{
	cpu_flags_t flags = ArchDisableInterrupts();

	HV_SIGNAL_EVENT_INPUT* input = (HV_SIGNAL_EVENT_INPUT*)PostMessagePage;
	input->ConnectionId.AsUint32 = ConnectionId.AsUint32;
	input->Flag = FlagNumber;

	HV_HYPERCALL_INPUT_VALUE inputValue = { 0 };
	inputValue.CallCode = HypercallCodes::SignalEvent;
	inputValue.Fast = true;

	Hypercall entry = (Hypercall)(void*)HypercallPage;

	HV_HYPERCALL_RESULT_VALUE status = { 0 };
	status.AsUint64 = entry(inputValue.AsUint64, input->AsUint64, NULL);

	ArchRestoreFlags(flags);
	return status;
}

void HyperV::ProcessInterrupts(uint32_t sint, std::list<uint32_t>& channelIds, std::list<HV_MESSAGE>& queue)
{
	cpu_flags_t flags = ArchDisableInterrupts();
	HV_SYNIC_EVENT_FLAGS* event = (HV_SYNIC_EVENT_FLAGS*)SynicEvents + sint;

	//Scan for bits
	for (uint32_t i = 0; i < HV_EVENT_FLAGS_COUNT; i++)
	{
		const uint8_t index = i / 8;
		const uint8_t flag = i % 8;

		const uint8_t mask = (1 << flag);
		if ((event->Flags[index] & mask) == 0)
			continue;

		const uint8_t cleared = event->Flags[index] & ~mask;

		//Clear flag
		_InterlockedCompareExchange((volatile long*)&event->Flags[index], cleared, event->Flags[index]);

		channelIds.push_back(i);
	}

	HV_MESSAGE* message = (HV_MESSAGE*)SynicMessages + sint;
	Assert((message->Header.MessageType & HV_MESSAGE_TYPE_HYPERVISOR_MASK) == 0);
	Assert(message->Header.MessageType < (size_t)vmbus_channel_message_type::CHANNELMSG_COUNT);

	if (message->Header.MessageType != HV_MESSAGE_TYPE::HvMessageTypeNone)
	{
		Assert((message->Header.MessageType & HV_MESSAGE_TYPE_HYPERVISOR_MASK) == 0);
		queue.push_back(*message);

		_InterlockedCompareExchange((volatile long*)& message->Header.MessageType, HvMessageTypeNone, message->Header.MessageType);

		if (message->Header.MessageFlags.MessagePending)
			HyperV::SignalEom();
	}

	ArchRestoreFlags(flags);
}


