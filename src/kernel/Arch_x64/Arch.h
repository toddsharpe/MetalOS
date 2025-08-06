#pragma once

#include "x64/x64.h"
#include "x64/CpuId.h"
#include "x64/PageTablesPool.h"
#include "x64/PageTables.h"
#include "x64/intrin.h"
#include "kernel/Arch_x64/x64Data.h"

/*
 * Forward x64 definitions
 */
typedef x64::InterruptVector InterruptVector;
typedef x64::InterruptFrame InterruptFrame;
typedef x64::SyscallFrame SyscallFrame;
typedef x64::Context Context;
static constexpr size_t IrqN = x64::IdtCount;
static constexpr size_t PageSize = x64::PageSize;
static constexpr size_t PageShift = x64::PageShift;
static constexpr size_t PageMask = x64::PageMask;

/*
 * Asm declarations.
 */
extern "C"
{
	void _ltr(uint16_t entry);
	void _sti();
	void _pause();
	void UpdateSegments(uint16_t data_selector, uint16_t code_selector);
	void x64_SYSTEMCALL();
	void ArchEnableInterrupts();
	cpu_flags_t ArchDisableInterrupts();
	void ArchRestoreFlags(const cpu_flags_t flags);
}

extern "C" void ArchInitialize()
{
	//Load new segments
	_lgdt(&x64::GDTR);

	//Update segment registers
	constexpr x64::SegmentSelector dataSelector(x64::GdtIndex::KernelData);
	constexpr x64::SegmentSelector codeSelector(x64::GdtIndex::KernelCode);
	UpdateSegments(dataSelector.Value, codeSelector.Value);

	constexpr x64::SegmentSelector tssSelector(x64::GdtIndex::TssEntry);
	_ltr(tssSelector.Value);

	//Load interrupt handlers
	__lidt(&x64::IDTR);

	//Enable interrupts
	_sti();

	//Enable syscalls
	constexpr x64::SegmentSelector userCodeSelector(x64::GdtIndex::User32Code, x64::UserDpl);
	const x64::IA32_STAR_REG starReg = { {0, codeSelector.Value, userCodeSelector.Value } };
	__writemsr(static_cast<uint32_t>(x64::MSR::IA32_STAR), starReg.AsUint64);
	__writemsr(static_cast<uint32_t>(x64::MSR::IA32_LSTAR), (uintptr_t)&x64_SYSTEMCALL);
	__writemsr(static_cast<uint32_t>(x64::MSR::IA32_FMASK), 0x200 | 0x100); //Disable interrupts and traps

	//Enable syscall
	const size_t value = __readmsr(static_cast<uint32_t>(x64::MSR::IA32_EFER));
	__writemsr(static_cast<uint32_t>(x64::MSR::IA32_EFER), value | 1);

	//Enable WRGSBASE instruction
	__writecr4(__readcr4() | (1 << 16));
}

size_t ArchStackReserve()
{
	return 32;
}

void ArchSetPagingRoot(paddr_t root)
{
	if (__readcr3() != root)
	{
		__writecr3(root);
	}
}

void ArchSetInterruptStack(void* stack)
{
	x64::TSS64.RSP_0_low = QWordLow(stack);
	x64::TSS64.RSP_0_high = QWordHigh(stack);
}

void ArchSetUserCpuContext(void* context)
{
	_writegsbase_u64((uintptr_t)context);
}

void ArchWait()
{
	__halt();
}

uint32_t ArchReadPort(uint32_t port, uint8_t width)
{
	switch (width)
	{
	case 8:
		return __inbyte(port);
	case 16:
		return __inword(port);
	case 32:
		return __indword(port);
	default:
		return 0;
	}
}

void ArchWritePort(uint32_t port, uint32_t value, uint8_t width)
{
	switch (width)
	{
	case 8:
		return __outbyte(port, value);
	case 16:
		return __outword(port, value);
	case 32:
		return __outdword(port, value);
	}
}
