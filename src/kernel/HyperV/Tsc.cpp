#include "kernel/HyperV/Tsc.h"

namespace HyperV::Tsc
{
	//12.7.3.3 Reference TSC during Save/Restore and Migration
	nano100_t ReadTsc()
	{
		uint64_t Scale, Offset;
		uint32_t StartSequence, EndSequence;
		uint64_t Tsc;
		uint64_t ReferenceTime;

		do
		{ 
			StartSequence = TscPage.TscSequence;
			if (StartSequence == 0)
			{ 
				// 0 means that the Reference TSC enlightenment is not available at  
				// the moment, and the Reference Time can only be obtained from  
				// reading the Reference Counter MSR. 
				ReferenceTime = __readmsr(static_cast<uint32_t>(MSR::HV_X64_MSR_TIME_REF_COUNT));
				return ReferenceTime; 
			} 
		
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
		ReferenceTime = highProduct + Offset; 
		return ReferenceTime;
	}

	void Initialize()
	{
		//Enable TSC
		HV_REF_TSC_REG tscReg = { 0 };
		tscReg.AsUint64 = __readmsr(static_cast<uint32_t>(MSR::HV_X64_MSR_REFERENCE_TSC));
		tscReg.Enable = true;
		tscReg.GPAPageNumber = ResolveImageVA((void*)&TscPage) >> PageShift;

		__writemsr(static_cast<uint32_t>(MSR::HV_X64_MSR_REFERENCE_TSC), tscReg.AsUint64);
		Printf("  Tsc: 0x%016x\n", ReadTsc());
	}
}
