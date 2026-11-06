#pragma once

#include "core_crt/stdint.h"
#include "windows/types.h"
#include "windows/winnt.h"
#include "kernel/Arch_x64/RuntimeSupport.h"
#include "x64/x64.h"

// https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=vs-2019
// http://www.nynaeve.net/Code/StackWalk64.cpp
class StackWalk
{
public:
	// ImageBase should probably be retired once kernel knows IP->Module lookups
	StackWalk(PCONTEXT context, const AddressSpace* const space = nullptr, PKNONVOLATILE_CONTEXT_POINTERS contextPointers = nullptr) : m_context(context),
																							m_contextPointers(contextPointers),
																							m_space(space),
																							m_unwound(),
																							m_own()
	{
		// Assert(context->Rsp);
		// Assert(context->Rbp);
		// Assert(context->Rip);
	}

	//Walks from a kernel context, unwinding a copy of it
	StackWalk(const x64::Context& context, const AddressSpace* const space) : m_context(&m_own),
																							m_contextPointers(),
																							m_space(space),
																							m_unwound(),
																							m_own()
	{
		m_own.Rip = context.Rip;
		m_own.Rsp = context.Rsp;
		m_own.Rbp = context.Rbp;
	}

	uintptr_t Ip() const
	{
		return m_context->Rip;
	}

	bool HasNext()
	{
		return m_context->Rip != NULL;
	}

	//Where to look the frame's module and line up: a caller's IP is the return address after its call, which can be on the next line (or past its function)
	uintptr_t LookupIp() const
	{
		return m_unwound ? m_context->Rip - 1 : m_context->Rip;
	}
	
	//False when the walk can't go on: the stack or the image's unwind data isn't readable, or isn't usable
	bool Next(const uintptr_t imageBase)
	{
		m_unwound = true;

		const RUNTIME_FUNCTION* functionEntry = RuntimeSupport::LookupFunctionEntry(m_context->Rip, imageBase, nullptr, m_space);
		if (!functionEntry)
		{
			// Pop IP off stack
			if (!RuntimeSupport::IsReadable(m_space, m_context->Rsp, sizeof(DWORD64)))
				return false;
			m_context->Rip = *(DWORD64 *)m_context->Rsp;
			m_context->Rsp += sizeof(DWORD64);
			return true;
		}
		return RuntimeSupport::VirtualUnwind(UNW_FLAG_NHANDLER, imageBase, m_context->Rip, functionEntry, m_context, m_contextPointers, m_space);
	}

private:
	PCONTEXT m_context;
	PKNONVOLATILE_CONTEXT_POINTERS m_contextPointers;
	const AddressSpace* const m_space; //A user stack and image are user memory, so each read is checked against the process's space; null for a kernel walk
	bool m_unwound; //Past the first frame (the faulting or captured IP), so RIP is a return address
	CONTEXT m_own; //The unwind context when walking from an x64::Context
};
