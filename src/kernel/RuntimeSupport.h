#pragma once


#include <efi.h>
#include <cstdint>

#include <string>
#include <windows/types.h>
#include <windows/winnt.h>
#include <coreclr/win64unwind.h>

//https://stackoverflow.com/questions/5705650/stackwalk64-on-windows-get-symbol-name
class RuntimeSupport
{
public:
	enum x64_NV_REG_NUM
	{
		Rax,
		Rcx,
		Rdx,
		Rbx,
		Rsp,
		Rbp,
		Rsi,
		Rdi,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15
	};

	static PRUNTIME_FUNCTION LookupFunctionEntry(const uint64_t ControlPC, const uintptr_t ImageBase, const void* HistoryTable);
	static PUNWIND_INFO GetUnwindInfo(const uintptr_t ImageBase, const PRUNTIME_FUNCTION function);
	static PUNWIND_INFO GetUnwindInfo(const uintptr_t address);
	static void VirtualUnwind(DWORD HandlerType, uintptr_t ImageBase, uintptr_t ControlPC, PRUNTIME_FUNCTION FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers);
	static void UnwindPrologue(const uintptr_t ImageBase, const uintptr_t ControlPc, const uintptr_t FrameBase, PRUNTIME_FUNCTION FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers);
};

