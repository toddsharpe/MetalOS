#pragma once

#include "Lib/Time.h"
#include "HyperV/HyperV.h"
#include "kernel/Arch.h"
#include "kernel/Api.h"
#include "Assert.h"

// Hypervisor Top Level Functional Specification v6.0b.pdf

namespace HyperV::Tsc
{
	//12.7.1 Reference Time Stamp Counter (TSC) Page MSR
	struct HV_REF_TSC_REG
	{
		union
		{
			struct
			{
				uint64_t Enable : 1;
				uint64_t Reserved : 11;
				uint64_t GPAPageNumber : 52;
			};
			uint64_t AsUint64;
		};
	};
	
	// 12.7.2 Format of the Reference TSC Page
	typedef struct _HV_REFERENCE_TSC_PAGE
	{
		volatile uint32_t TscSequence;
		uint32_t Reserved1;
		volatile uint64_t TscScale;
		volatile int64_t TscOffset;
		uint64_t Reserved2[509];
	} HV_REFERENCE_TSC_PAGE, *PHV_REFERENCE_TSC_PAGE;
	volatile HYPERV_PAGE_ALIGN HV_REFERENCE_TSC_PAGE TscPage = {0};

	void Initialize();

	//12.7.3.3 Reference TSC during Save/Restore and Migration
	nano100_t ReadTsc();
}