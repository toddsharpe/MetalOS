#pragma once

#include "core_crt/stdint.h"
#include "kernel/Arch.h"
#include "kernel/Plat_HyperV/Types.h"

// Hypervisor Top Level Functional Specification v6.0b.pdf

namespace HyperV::Platform
{
	void Initialize();
	HV_HYPERCALL_RESULT_VALUE HvPostMessage(HV_CONNECTION_ID ConnectionId, HV_MESSAGE_TYPE MessageType, uint32_t PayloadSize, const void* Message);
	HV_HYPERCALL_RESULT_VALUE HvSignalEvent(HV_CONNECTION_ID ConnectionId, uint16_t FlagNumber);
	void SignalEom();
}
