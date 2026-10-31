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

		//Clear each set bit with an atomic test-and-reset: the host sets bits concurrently and a byte RMW could drop one
		HV_SYNIC_EVENT_FLAGS* event = (HV_SYNIC_EVENT_FLAGS*)SynicEvents + sint;
		volatile long* const words = reinterpret_cast<volatile long*>(event->Flags);
		for (uint32_t w = 0; w < HV_EVENT_FLAGS_COUNT / 32; w++)
		{
			const uint32_t pending = static_cast<uint32_t>(words[w]);
			if (pending == 0)
				continue;

			for (uint32_t bit = 0; bit < 32; bit++)
			{
				if (((pending >> bit) & 1) && _interlockedbittestandreset(&words[w], bit))
					channelIds.Add(w * 32 + bit);
			}
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
