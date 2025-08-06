#pragma once

#include "core_crt/stdint.h"
#include "x64/intrin.h"

namespace CpuId
{
	struct Result
	{
		uint32_t EAX;
		uint32_t EBX;
		uint32_t ECX;
		uint32_t EDX;
	};

	const char* GetVendor();
	bool IsHypervisor();
	void Get(Result& result, const int function);
}
