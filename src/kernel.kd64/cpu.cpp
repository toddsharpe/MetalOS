#include <sal.h>
#include <cstdint>
#include <intrin.h>
#include <ntstatus.h>
#include <windows/types.h>
#include <reactos/ioaccess.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include <kernel/kddll.h>
#include <kernel/MetalOS.Arch.h>
#include <kernel/MetalOS.Internal.h>
#include <reactos/amd64/ke.h>
#include <reactos/ke.h>
#include "kd64.h"

VOID
NTAPI
KiRestoreProcessorControlState(PKPROCESSOR_STATE ProcessorState)
{
	/* Restore the CR registers */
	__writecr0(ProcessorState->SpecialRegisters.Cr0);
	//    __writecr2(ProcessorState->SpecialRegisters.Cr2);
	__writecr3(ProcessorState->SpecialRegisters.Cr3);
	__writecr4(ProcessorState->SpecialRegisters.Cr4);
	__writecr8(ProcessorState->SpecialRegisters.Cr8);

	/* Restore the DR registers */
	__writedr(0, ProcessorState->SpecialRegisters.KernelDr0);
	__writedr(1, ProcessorState->SpecialRegisters.KernelDr1);
	__writedr(2, ProcessorState->SpecialRegisters.KernelDr2);
	__writedr(3, ProcessorState->SpecialRegisters.KernelDr3);
	__writedr(6, ProcessorState->SpecialRegisters.KernelDr6);
	__writedr(7, ProcessorState->SpecialRegisters.KernelDr7);

	/* Restore GDT, IDT, LDT and TSS */
	_lgdt(&ProcessorState->SpecialRegisters.Gdtr.Limit);
	//    __lldt(&ProcessorState->SpecialRegisters.Ldtr);
	//    __ltr(&ProcessorState->SpecialRegisters.Tr);
	__lidt(&ProcessorState->SpecialRegisters.Idtr.Limit);

	//    __ldmxcsr(&ProcessorState->SpecialRegisters.MxCsr); // FIXME
	//    ProcessorState->SpecialRegisters.DebugControl
	//    ProcessorState->SpecialRegisters.LastBranchToRip
	//    ProcessorState->SpecialRegisters.LastBranchFromRip
	//    ProcessorState->SpecialRegisters.LastExceptionToRip
	//    ProcessorState->SpecialRegisters.LastExceptionFromRip

		/* Restore MSRs */
	__writemsr(X86_MSR_GSBASE, ProcessorState->SpecialRegisters.MsrGsBase);
	__writemsr(X86_MSR_KERNEL_GSBASE, ProcessorState->SpecialRegisters.MsrGsSwap);
	__writemsr(X86_MSR_STAR, ProcessorState->SpecialRegisters.MsrStar);
	__writemsr(X86_MSR_LSTAR, ProcessorState->SpecialRegisters.MsrLStar);
	__writemsr(X86_MSR_CSTAR, ProcessorState->SpecialRegisters.MsrCStar);
	__writemsr(X86_MSR_SFMASK, ProcessorState->SpecialRegisters.MsrSyscallMask);

}

VOID
NTAPI
KiSaveProcessorControlState(OUT PKPROCESSOR_STATE ProcessorState)
{
	/* Save the CR registers */
	ProcessorState->SpecialRegisters.Cr0 = __readcr0();
	ProcessorState->SpecialRegisters.Cr2 = __readcr2();
	ProcessorState->SpecialRegisters.Cr3 = __readcr3();
	ProcessorState->SpecialRegisters.Cr4 = __readcr4();
	ProcessorState->SpecialRegisters.Cr8 = __readcr8();

	/* Save the DR registers */
	ProcessorState->SpecialRegisters.KernelDr0 = __readdr(0);
	ProcessorState->SpecialRegisters.KernelDr1 = __readdr(1);
	ProcessorState->SpecialRegisters.KernelDr2 = __readdr(2);
	ProcessorState->SpecialRegisters.KernelDr3 = __readdr(3);
	ProcessorState->SpecialRegisters.KernelDr6 = __readdr(6);
	ProcessorState->SpecialRegisters.KernelDr7 = __readdr(7);

	/* Save GDT, IDT, LDT and TSS */
	_sgdt(&ProcessorState->SpecialRegisters.Gdtr.Limit);
	//__sldt(&ProcessorState->SpecialRegisters.Ldtr);
	//__str(&ProcessorState->SpecialRegisters.Tr);
	__sidt(&ProcessorState->SpecialRegisters.Idtr.Limit);

	//    __stmxcsr(&ProcessorState->SpecialRegisters.MxCsr);
	//    ProcessorState->SpecialRegisters.DebugControl =
	//    ProcessorState->SpecialRegisters.LastBranchToRip =
	//    ProcessorState->SpecialRegisters.LastBranchFromRip =
	//    ProcessorState->SpecialRegisters.LastExceptionToRip =
	//    ProcessorState->SpecialRegisters.LastExceptionFromRip =

		/* Save MSRs */
	ProcessorState->SpecialRegisters.MsrGsBase = __readmsr(X86_MSR_GSBASE);
	ProcessorState->SpecialRegisters.MsrGsSwap = __readmsr(X86_MSR_KERNEL_GSBASE);
	ProcessorState->SpecialRegisters.MsrStar = __readmsr(X86_MSR_STAR);
	ProcessorState->SpecialRegisters.MsrLStar = __readmsr(X86_MSR_LSTAR);
	ProcessorState->SpecialRegisters.MsrCStar = __readmsr(X86_MSR_CSTAR);
	ProcessorState->SpecialRegisters.MsrSyscallMask = __readmsr(X86_MSR_SFMASK);
}
