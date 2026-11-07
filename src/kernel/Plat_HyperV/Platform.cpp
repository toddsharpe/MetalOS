#include "kernel/Plat_HyperV/Platform.h"
#include "Assert.h"
#include "kernel/Kernel_vmem.h"

namespace HyperV::Platform
{
	//Code page, map as execute
	static volatile HYPERV_PAGE_ALIGN uint8_t HypercallPage[Arch::PageSize] = { };
	static Hypercall hypercall = (Hypercall)(void*)HypercallPage;

	static volatile HYPERV_PAGE_ALIGN uint8_t PostMessagePage[Arch::PageSize] = {};

	static char Vendor[13] = { 0 };

	//TLFS: 3.13
	void Initialize()
	{
		//1. Verify a hypervisor is present
		Assert(Arch::CpuId::IsHypervisor());

		//2. Read highest leaf and interface signature
		{
			//Highest leaf value
			Arch::CpuId::Result result = {};
			Arch::CpuId::Get(result, static_cast<uint32_t>(CPUID_LEAF::Vendor));
			Assert(result.EAX >= static_cast<uint32_t>(CPUID_LEAF::Highest));

			//Get vendor name
			*((uint32_t*)Vendor) = (uint32_t)result.EBX;
			*((uint32_t*)(Vendor + sizeof(uint32_t))) = (uint32_t)result.ECX;
			*((uint32_t*)(Vendor + sizeof(uint32_t) * 2)) = (uint32_t)result.EDX;
			Assert(strcmp(Vendor, "Microsoft Hv") == 0);

			//Interface
			Arch::CpuId::Get(result, static_cast<uint32_t>(CPUID_LEAF::Interface));
			char interface[5] = { 0 };
			*((uint32_t*)interface) = (uint32_t)result.EAX;
			Assert(strcmp(interface, "Hv#1") == 0);
		}

		//3. Write identity. Specify Linux
		GUEST_OS_ID_REG guestId = {};
		guestId.BuildNumber = 0x02;
		guestId.Version = 0x01;
		guestId.OSID = 0x1;
		guestId.OSType = GUEST_OS_TYPE::Linux;
		guestId.OpenSource = true;
		ArchWriteMsr(static_cast<uint32_t>(MSR::HV_X64_MSR_GUEST_OS_ID), guestId.Value);

		//4. Read Hypercall MSR
		{
			HYPERCALL_REG reg = { };
			reg.AsUint64 = ArchReadMsr(static_cast<uint32_t>(MSR::HV_X64_MSR_HYPERCALL));
		
			//5. Check if interface is already enabled
			if (!reg.Enabled)
			{
				//6. Page is preallocated by loader

				//7. Write to hypercall reg
				reg.Enabled = true;
				reg.HypercallGPFN = ResolveImageVA((void*)HypercallPage) >> Arch::PageShift;
				ArchWriteMsr(static_cast<uint32_t>(MSR::HV_X64_MSR_HYPERCALL), reg.AsUint64);
			}
		}


		//8. Page is mapped by loader
	}


	//Code: 0x005C
	//Params:
	//0 - ConnectionId (4 bytes), Padding (4 bytes) - from HvConnectPort
	//8 - MessageType (4 bytes), PayloadSize (4 bytes) - TopBit of MessageType is cleared. PayloadSize (up to 240 bytes)
	//16 Message[0] (8 bytes)
	//248 Message[29] (8 bytes)
	HV_HYPERCALL_RESULT_VALUE HvPostMessage(
		HV_CONNECTION_ID ConnectionId, 
		HV_MESSAGE_TYPE MessageType,
		uint32_t PayloadSize,
		const void* Message
	)
	{
		Assert(PayloadSize <= HV_MESSAGE_PAYLOAD_BYTE_COUNT);
		
		cpu_flags_t flags = ArchDisableInterrupts();
		
		HV_POST_MESSAGE_INPUT* input = (HV_POST_MESSAGE_INPUT*)PostMessagePage;
		input->ConnectionId = ConnectionId;
		input->MessageType = MessageType;
		input->PayloadSize = PayloadSize;
		memcpy(input->Payload, Message, PayloadSize);

		HV_HYPERCALL_INPUT_VALUE inputValue = { };
		inputValue.CallCode = HYPERCALL_CODE::HvPostMessage;

		HV_HYPERCALL_RESULT_VALUE status = { 0 };
		status.AsUint64 = hypercall(inputValue.AsUint64, ResolveImageVA((void*)input), NULL);

		ArchRestoreFlags(flags);
		return status;
	}

	//Code: 0x5D
	//0 - ConnectionId (4 bytes), FlagNumber (2 bytes), Reserved (2 bytes)
	HV_HYPERCALL_RESULT_VALUE HvSignalEvent(
		HV_CONNECTION_ID ConnectionId,
		uint16_t FlagNumber //relative to base number for port
	)
	{
		cpu_flags_t flags = ArchDisableInterrupts();

		HV_SIGNAL_EVENT_INPUT* input = (HV_SIGNAL_EVENT_INPUT*)PostMessagePage;
		input->ConnectionId.AsUint32 = ConnectionId.AsUint32;
		input->Flag = FlagNumber;

		HV_HYPERCALL_INPUT_VALUE inputValue = {};
		inputValue.CallCode = HYPERCALL_CODE::HvSignalEvent;
		inputValue.Fast = true;

		HV_HYPERCALL_RESULT_VALUE status = { 0 };
		status.AsUint64 = hypercall(inputValue.AsUint64, input->AsUint64, NULL);

		ArchRestoreFlags(flags);
		return status;
	}

	void SignalEom()
	{
		ArchWriteMsr(static_cast<uint32_t>(MSR::EOM), 0);
	}
}
