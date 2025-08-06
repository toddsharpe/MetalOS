#pragma once

#include "kernel/HyperV/Types.h"
#include "Lib/StaticVector.h"

namespace HyperV::Interrupts
{
	void Initialize();

	void EnableSintVector(const uint32_t sint, const uint32_t vector);

	//TODO(tsharpe): Make method not depend on size of container
	static constexpr size_t MaxChannelIds = 32;
	void ProcessInterrupts(uint32_t sint, StaticVector<uint32_t, MaxChannelIds>& channelIds, HV_MESSAGE& messageOut);

	void EOI();
}
