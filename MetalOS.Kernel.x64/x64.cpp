#include "x64.h"

#include "Support.h"
#include "SystemCall.h"
#include "msvc.h"
#include <intrin.h>

void x64::SetUserCpuContext(void* context)
{
	_writegsbase_u64((uintptr_t)context);
}

void x64::SetKernelCpuContext(void* context)
{
	__writemsr(static_cast<uint32_t>(MSR::IA32_KERNELGS_BASE), (uintptr_t)context);
}

void x64::SetKernelInterruptStack(void* stack)
{
	TSS64.RSP_0_low = QWordLow(stack);
	TSS64.RSP_0_high = QWordHigh(stack);
}

void x64::Initialize()
{
	//Load new segments
	_lgdt(&GDTR);

	//Update segment registers
	SEGMENT_SELECTOR dataSelector(static_cast<uint16_t>(GDT::KernelData));
	SEGMENT_SELECTOR codeSelector(static_cast<uint16_t>(GDT::KernelCode));
	UpdateSegments(dataSelector.Value, codeSelector.Value);

	SEGMENT_SELECTOR tssSelector(static_cast<uint16_t>(GDT::TssEntry));
	_ltr(tssSelector.Value);

	//Load interrupt handlers
	__lidt(&IDTR);

	//Enable interrupts
	_sti();

	//Enable syscalls
	SEGMENT_SELECTOR userCodeSelector(static_cast<uint16_t>(GDT::User32Code), UserDPL);
	IA32_STAR_REG starReg = { 0 };
	starReg.SyscallCS = codeSelector.Value;
	starReg.SysretCS = userCodeSelector.Value;
	__writemsr(static_cast<uint32_t>(MSR::IA32_STAR), starReg.AsUint64);
	__writemsr(static_cast<uint32_t>(MSR::IA32_LSTAR), (uintptr_t)&x64_SYSTEMCALL);
	__writemsr(static_cast<uint32_t>(MSR::IA32_FMASK), 0x200 | 0x100); //Disable interrupts and traps

	//Enable syscall
	const size_t value = __readmsr(static_cast<uint32_t>(MSR::IA32_EFER));
	__writemsr(static_cast<uint32_t>(MSR::IA32_EFER), value | 1);

	//Enable WRGSBASE instruction
	__writecr4(__readcr4() | (1 << 16));
}
