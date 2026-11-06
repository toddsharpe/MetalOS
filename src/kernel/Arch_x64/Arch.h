#pragma once

#include "x64/x64.h"
#include "x64/Types.h"
#include "x64/CpuId.h"
#include "x64/PageTables.h"
#include "x64/intrin.h"
#include "Assert.h"

/*
 * Forward x64 definitions
 */
namespace Arch
{
	using InterruptVector = x64::InterruptVector;
	using InterruptFrame = x64::InterruptFrame;
	using SyscallFrame = x64::SyscallFrame;
	using Context = x64::Context;
	static constexpr size_t IrqN = x64::IdtCount;
	static constexpr size_t PageSize = x64::PageSize;
	static constexpr size_t PageShift = x64::PageShift;
	static constexpr size_t PageMask = x64::PageMask;

	constexpr size_t SizeToPages(const size_t bytes)
	{
		return x64::SizeToPages(bytes);
	}

	//Page tables, and the attributes of the kernel's and a user process's mappings
	using PageTablesOps = ::PageTablesOps;
	template <PageTablesOps Ops>
	using PageTables = PageTabes<Ops>;
	using PageAttr = ::PageAttr;
	static constexpr PageAttr KernelPages = KernelAll;
	static constexpr PageAttr UserPages = UserAll;

	//A thread's FPU/SSE state
	using fpu_context_t = x64::FxSave;

	namespace CpuId = ::CpuId;
}

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

//Home area plus the return-address slot of an entry point reached by jmp or sysret rather than call
inline size_t ArchStackReserve()
{
	return 40;
}

inline void ArchSetPagingRoot(paddr_t root)
{
	if (__readcr3() != root)
	{
		__writecr3(root);
	}
}

void ArchSetInterruptStack(void* stack);

inline void ArchSetUserCpuContext(void* context)
{
	_writegsbase_u64((uintptr_t)context);
}

inline void ArchSetUserGsBase(void* teb)
{
	__writemsr(static_cast<uint32_t>(x64::MSR::IA32_KERNELGS_BASE), (uintptr_t)teb);
}

inline void ArchWait()
{
	__halt();
}

inline uint32_t ArchReadPort(uint32_t port, uint8_t width)
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

inline void ArchWritePort(uint32_t port, uint32_t value, uint8_t width)
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

inline uint64_t ArchReadMsr(const uint32_t msr)
{
	return __readmsr(msr);
}

inline void ArchWriteMsr(const uint32_t msr, const uint64_t value)
{
	__writemsr(msr, value);
}

inline uint64_t ArchReadTimestamp()
{
	return __rdtsc();
}

//Orders every earlier memory access before every later one, for memory shared with the host or a device
inline void ArchMemoryBarrier()
{
	__faststorefence();
}

//The address the last page fault touched
inline uintptr_t ArchFaultAddress()
{
	return __readcr2();
}

//Sets a zeroed FPU/SSE state to the power-on defaults: x87 control word 0x37F and MXCSR 0x1F80, all exceptions masked
inline void ArchInitFpu(Arch::fpu_context_t& state)
{
	state.ControlWord = 0x037F;
	state.MxCsr = 0x1F80;
}

inline void ArchSaveFpu(Arch::fpu_context_t& state)
{
	_fxsave64(&state);
}

inline void ArchLoadFpu(const Arch::fpu_context_t& state)
{
	_fxrstor64(&state);
}

//A stack walk's starting point from its instruction, stack and frame pointers
inline Arch::Context ArchMakeContext(const uintptr_t ip, const uintptr_t sp, const uintptr_t fp)
{
	Arch::Context context = {};
	context.Rip = ip;
	context.Rsp = sp;
	context.Rbp = fp;
	return context;
}

//A stack walk's starting point from the code an interrupt stopped
inline Arch::Context ArchFrameContext(const Arch::InterruptFrame& frame)
{
	return ArchMakeContext(frame.RIP, frame.RSP, frame.RBP);
}

inline uint64_t ArchFrameErrorCode(const Arch::InterruptFrame& frame)
{
	return frame.ErrorCode;
}

inline void ArchPrintFrame(const Arch::InterruptFrame& frame)
{
	Printf("    RIP: 0x%016x\n", frame.RIP);
	Printf("    RBP: 0x%016x\n", frame.RBP);
	Printf("    RSP: 0x%016x\n", frame.RSP);
	Printf("    RAX: 0x%016x\n", frame.RAX);
	Printf("    RBX: 0x%016x\n", frame.RBX);
	Printf("    RCX: 0x%016x\n", frame.RCX);
	Printf("    RDX: 0x%016x\n", frame.RDX);
	Printf("    CS: 0x%x, SS: 0x%x\n", frame.CS, frame.SS);
}

inline void ArchPrintContext(const Arch::Context& context)
{
	Printf("  Rbp: 0x%016x Rsp: 0x%016x Rip: 0x%016x\n", context.Rbp, context.Rsp, context.Rip);
}
