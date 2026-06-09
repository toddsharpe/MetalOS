#include "kernel/HyperV/Interrupts.h"
#include "kernel/HyperV/Platform.h"
#include "kernel/Api.h"

namespace HyperV::Interrupts
{
	// examine flags, clear using LOCK AND or LOCK CMPXCHG, write EOI, process
	volatile HYPERV_PAGE_ALIGN HV_SYNIC_EVENT_FLAGS SynicEvents[HV_SYNIC_SINT_COUNT] = { };

	// examine, copy to another location (work queue), set type to None, write EOI, and then process
	volatile HYPERV_PAGE_ALIGN HV_MESSAGE SynicMessages[HV_SYNIC_SINT_COUNT] = { HV_MESSAGE_TYPE::HvMessageTypeNone, 0 };
	
	void Initialize()
	{
		// Initialize Synic Events/Messages
		{
			const paddr_t address = ResolveImageVA((void *)SynicEvents);

			HV_INTERRUPT_PAGE_REGISTER reg;
			reg.AsUint64 = __readmsr(static_cast<uint32_t>(MSR::SIEFP));
			reg.Enable = true;
			reg.BaseAddress = (address >> Arch::PageShift);
			__writemsr(static_cast<uint32_t>(MSR::SIEFP), reg.AsUint64);
		}
		{
			const paddr_t address = ResolveImageVA((void *)SynicMessages);

			HV_INTERRUPT_PAGE_REGISTER reg;
			reg.AsUint64 = __readmsr(static_cast<uint32_t>(MSR::SIMP));
			reg.Enable = true;
			reg.BaseAddress = (address >> Arch::PageShift);

			__writemsr(static_cast<uint32_t>(MSR::SIMP), reg.AsUint64);
		}

		// Enable interrupts
		{
			HV_SCONTROL_REGISTER reg;
			reg.AsUint64 = __readmsr(static_cast<uint32_t>(MSR::SCONTROL));
			reg.Enabled = true;
			__writemsr(static_cast<uint32_t>(MSR::SCONTROL), reg.AsUint64);
		}
	}

	void EnableSintVector(const uint32_t sint, const uint32_t vector)
	{
		const uint32_t msr = static_cast<uint32_t>(MSR::SINT0) + sint;

		HV_SINT_REGISTER reg = {0};
		reg.AsUint64 = __readmsr(msr);
		reg.Vector = vector;
		reg.Masked = false;
		reg.AutoEOI = false;
		__writemsr(msr, reg.AsUint64);
	}

	void ProcessInterrupts(uint32_t sint, StaticVector<uint32_t, MaxChannelIds>& channelIds, HV_MESSAGE& messageOut)
	{
		cpu_flags_t flags = ArchDisableInterrupts();

		//Scan event page
		HV_SYNIC_EVENT_FLAGS* event = (HV_SYNIC_EVENT_FLAGS*)SynicEvents + sint;
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

			channelIds.Add(i);
		}

		//Get corresponding message
		HV_MESSAGE* message = (HV_MESSAGE*)SynicMessages + sint;
		Assert(((uint32_t)message->Header.MessageType & HV_MESSAGE_TYPE_HYPERVISOR_MASK) == 0);

		if (message->Header.MessageType != HV_MESSAGE_TYPE::HvMessageTypeNone)
		{
			messageOut = *message;

			_InterlockedCompareExchange((volatile long*)& message->Header.MessageType, (uint32_t)HV_MESSAGE_TYPE::HvMessageTypeNone, (uint32_t)message->Header.MessageType);

			if (message->Header.MessageFlags.MessagePending)
				HyperV::Platform::SignalEom();
		}

		ArchRestoreFlags(flags);
	}

	void EOI()
	{
		__writemsr(static_cast<uint32_t>(MSR::HV_X64_MSR_EOI), 0);
	}
}
