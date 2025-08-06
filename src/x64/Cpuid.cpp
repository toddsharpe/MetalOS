#pragma once

#include "x64/Cpuid.h"

namespace CpuId
{
	enum Regs
	{
		EAX,
		EBX,
		ECX,
		EDX
	};
	
	const char* GetVendor()
	{
		static bool init = false;
		static char vendor[13] = { };

		if (!init)
		{
			int registers[4] = { };
			__cpuid(registers, 0);

			uint32_t* const words = reinterpret_cast<uint32_t*>(vendor);
			words[0] = static_cast<uint32_t>(registers[Regs::EBX]);
			words[1] = static_cast<uint32_t>(registers[Regs::EDX]);
			words[2] = static_cast<uint32_t>(registers[Regs::ECX]);

			init = true;
		}
		return vendor;
	}

	bool IsHypervisor()
	{
		static bool init = false;
		static bool isHypervisor = false;

		if (!init)
		{
			int registers[4] = { };
			__cpuid(registers, 1);

			isHypervisor = ((uint32_t)registers[ECX] & (0x800000)) != 0;

			init = true;
		}
		return isHypervisor;
	}

	void Get(Result& result, const int function)
	{
		__cpuid((int*)&result, function);
	}
}
