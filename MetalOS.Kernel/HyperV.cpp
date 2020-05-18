#include "HyperV.h"

#include "Cpuid.h"
#include "Main.h"
#include <intrin.h>
#include "x64_support.h"

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

volatile HyperV::HV_REFERENCE_TSC_PAGE HyperV::TscPage = { 0 };

volatile HyperV::HV_MESSAGE HyperV::SynicMessages[HyperV::HV_SYNIC_SINT_COUNT];
volatile HyperV::HV_SYNIC_EVENT_FLAGS HyperV::SynicEvents[HyperV::HV_SYNIC_SINT_COUNT] = { 0 };
volatile uint8_t HyperV::HypercallPage[PAGE_SIZE] = { 0 };
volatile uint8_t HyperV::PostMessagePage[PAGE_SIZE];

volatile uint8_t HyperV::MonitorPage1[PAGE_SIZE] = { 0 };
volatile uint8_t HyperV::MonitorPage2[PAGE_SIZE] = { 0 };

HyperV::HyperV() :
	m_threadSignal(),
	m_queue()
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

	//Interface
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
}

//HV TLFS 3.13
void HyperV::Initialize()
{
	Print("HyperV::Initialize\n");

	Assert(this->IsPresent());
	Assert(this->DirectSyntheticTimers());
	Assert(this->AccessPartitionReferenceCounter());
	
	//Allocate resources
	//TODO: allocate resources on the fly versus our current static allocated page
	m_threadSignal = kernel.CreateSemaphore(0, 0, "HyperVThread");
	m_connectSemaphore = kernel.CreateSemaphore(0, 0, "HyperVConnect");
	kernel.CreateThread(HyperV::ThreadLoop, this);

	//1. Asserted by CpuId

	//2. Asserted by HyperV()

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

	//Enable Synic Message Page
	SetSynicMessagePage(kernel.VirtualToPhysical((paddr_t)SynicMessages));

	//Enable Synic Event Page
	SetSynicEventPage(kernel.VirtualToPhysical((paddr_t)SynicEvents));

	//Register SINT
	{
		HV_SINT_REGISTER reg = { 0 };
		reg.AsUint64 = ReadSintRegister(VMBUS_SINT::VMBUS_MESSAGE_SINT);
		reg.Vector = HypervisorCallback;
		reg.Masked = false;
		reg.AutoEOI = !DeprecateAutoEOI();
		WriteSintRegister(VMBUS_SINT::VMBUS_MESSAGE_SINT, reg.AsUint64);
	}

	//Enable TSC
	HV_REF_TSC_REG tscReg = { 0 };
	tscReg.AsUint64 = __readmsr(HV_REG::HV_X64_MSR_REFERENCE_TSC);
	tscReg.Enable = true;
	tscReg.GPAPageNumber = kernel.VirtualToPhysical((uintptr_t)&TscPage) >> PAGE_SHIFT;
	__writemsr(HV_REG::HV_X64_MSR_REFERENCE_TSC, tscReg.AsUint64);
	Print("  Tsc: 0x%016x\n", ReadTsc());

	//Enable Synic
	EnableSynic();
}

void HyperV::Connect()
{
	//Initiate connection
	vmbus_channel_initiate_contact msg;
	memset(&msg, 0, sizeof(vmbus_channel_initiate_contact));
	msg.header.msgtype = CHANNELMSG_INITIATE_CONTACT;
	msg.vmbus_version_requested = VERSION_WIN10_V5_2;
	msg.msg_sint = VMBUS_SINT::VMBUS_MESSAGE_SINT;
	msg.monitor_page1 = kernel.VirtualToPhysical((uintptr_t)& MonitorPage1);
	msg.monitor_page2 = kernel.VirtualToPhysical((uintptr_t)& MonitorPage2);
	msg.target_vcpu = 0;

	HV_CONNECTION_ID connectionId = { 0 };
	connectionId.Id = VMBUS_MESSAGE_CONNECTION_ID_4;
	HV_HYPERCALL_RESULT_VALUE result = HvPostMessage(connectionId, (HV_MESSAGE_TYPE)1, sizeof(vmbus_channel_initiate_contact), &msg);
	Print("Result: %d\n", result.Status);
	kernel.WaitForSemaphore(m_connectSemaphore, INT64_MAX);

	Print("continue\n");
	connectionId.Id = m_msg_conn_id;
	Print("msg_conn_id %d\n", connectionId.Id);

	//vmbus_request_offers
	vmbus_channel_message_header header;
	memset(&header, 0, sizeof(vmbus_channel_message_header));
	header.msgtype = CHANNELMSG_REQUESTOFFERS;
	result = HvPostMessage(connectionId, (HV_MESSAGE_TYPE)1, sizeof(vmbus_channel_message_header), &header);
	Print("Result: %d\n", result.Status);
	kernel.WaitForSemaphore(m_connectSemaphore, INT64_MAX);

	Print("Continue2\n");
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

HyperV::HV_HYPERCALL_RESULT_VALUE HyperV::HvPostMessage(
	__in HV_CONNECTION_ID ConnectionId,
	__in HV_MESSAGE_TYPE MessageType,
	__in UINT32 PayloadSize,
	__in_ecount(PayloadSize)
	PCVOID Message
)
{
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
	return status;
}

//Check events then messages
void HyperV::OnInterrupt()
{
	cpu_flags_t flags = x64_disable_interrupts();
	Print("HyperV!\n");

	HV_SYNIC_EVENT_FLAGS* event = (HV_SYNIC_EVENT_FLAGS*)SynicEvents + VMBUS_SINT::VMBUS_MESSAGE_SINT;
	Print("Events: 0x%x\n", event);
	for (size_t i = 0; i < HV_EVENT_FLAGS_BYTE_COUNT; i++)
	{
		if (event->Flags[i] != 0)
			Print("[%d]=%02x ", event->Flags[i], i);
	}

	HV_MESSAGE* message = (HV_MESSAGE*)SynicMessages + VMBUS_SINT::VMBUS_MESSAGE_SINT;
	Assert((message->Header.MessageType & HV_MESSAGE_TYPE_HYPERVISOR_MASK) == 0);
	Assert(message->Header.MessageType < CHANNELMSG_COUNT);

	Print("Type: %d\n", message->Header.MessageType);
	if (message->Header.MessageType != HV_MESSAGE_TYPE::HvMessageTypeNone)
	{
		m_queue.push_back(*message);

		//Signal EOM
		_InterlockedCompareExchange((volatile long*)& message->Header.MessageType, HvMessageTypeNone, message->Header.MessageType);
		Print("   Type: %d\n", message->Header.MessageType);

		if (message->Header.MessageFlags.MessagePending)
			SignalEom();

		//Start processing thread
		kernel.ReleaseSemaphore(this->m_threadSignal, 1);
	}

	HyperV::EOI();
	x64_restore_flags(flags);
}

uint32_t HyperV::ThreadLoop(void* arg)
{
	HyperV* hyperV = (HyperV*)arg;
	while (true)
	{
		Print("HyperV::ThreadLoop\n");
		kernel.WaitForSemaphore(hyperV->m_threadSignal, INT64_MAX);

		Assert(!hyperV->m_queue.empty());

		//while (!hyperV->m_queue.empty())

		const HV_MESSAGE& message = hyperV->m_queue.front();
		hyperV->m_queue.pop_front();
		Print("Type: %d\n", message.Header.MessageType);

		vmbus_channel_message_header* header = (vmbus_channel_message_header*)message.Payload;
		Print("header: %d\n", header->msgtype);
		switch (header->msgtype)
		{
		case CHANNELMSG_VERSION_RESPONSE:
		{
			vmbus_channel_version_response* response = (vmbus_channel_version_response*)message.Payload;
			Assert(response->version_supported);
			Print("%d %d %d\n", response->version_supported, response->msg_conn_id, response->connection_state);
			hyperV->m_msg_conn_id = response->msg_conn_id;
		}
		break;
		case CHANNELMSG_OFFERCHANNEL:
		{
			vmbus_channel_offer_channel* offer = (vmbus_channel_offer_channel*)message.Payload;
			vmbus_channel_offer o = offer->offer;
			Assert(o.sub_channel_index == 0);

			Print("if_type ");
			Print("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX} ",
				o.if_type.Data1,
				o.if_type.Data2,
				o.if_type.Data3,
				o.if_type.Data4[0], o.if_type.Data4[1], o.if_type.Data4[2], o.if_type.Data4[3],
				o.if_type.Data4[4], o.if_type.Data4[5], o.if_type.Data4[6], o.if_type.Data4[7]);

		}
		break;
		}

		//Let connect proceed
		kernel.ReleaseSemaphore(hyperV->m_connectSemaphore, 1);
	}
}

/*
Connection order:
Guest: CHANNELMSG_INITIATE_CONTACT. Host: CHANNELMSG_VERSION_RESPONSE
Guest: CHANNELMSG_REQUESTOFFERS. Host: multiple CHANNELMSG_OFFERCHANNEL
*/
