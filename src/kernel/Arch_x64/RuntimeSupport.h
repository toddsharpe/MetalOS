#pragma once

#include <windows/types.h>
#include <windows/winnt.h>
#include <coreclr/win64unwind.h>
#include "kernel/Core/AddressSpace.h"

namespace RuntimeSupport
{
	//A user walk checks every read against the process's address space; a kernel walk (no space) reads unchecked
	bool IsReadable(const AddressSpace* const space, const uintptr_t address, const size_t length);

	const RUNTIME_FUNCTION* LookupFunctionEntry(const uint64_t ControlPC, const uintptr_t ImageBase, const void* HistoryTable, const AddressSpace* const Space = nullptr);
	bool VirtualUnwind(DWORD HandlerType, ULONG64 ImageBase, ULONG64 ControlPC, const RUNTIME_FUNCTION* FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers, const AddressSpace* const Space = nullptr);
}
