#pragma once

#include "core_crt/stdint.h"
#include "windows/types.h"
#include "windows/winnt.h"
#include "kernel/Arch_x64/RuntimeSupport.h"

// https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=vs-2019
// http://www.nynaeve.net/Code/StackWalk64.cpp
class StackWalk
{
public:
	// ImageBase should probably be retired once kernel knows IP->Module lookups
	StackWalk(PCONTEXT context, PKNONVOLATILE_CONTEXT_POINTERS contextPointers = nullptr) : m_context(context),
																							m_contextPointers(contextPointers)
	{
		// Assert(context->Rsp);
		// Assert(context->Rbp);
		// Assert(context->Rip);
	}

	bool HasNext()
	{
		return m_context->Rip != NULL;
	}
	
	PCONTEXT Next(const uintptr_t imageBase)
	{
		const RUNTIME_FUNCTION* functionEntry = RuntimeSupport::LookupFunctionEntry(m_context->Rip, imageBase, nullptr);
		if (!functionEntry)
		{
			// Pop IP off stack
			Printf("Got from stack\n");
			m_context->Rip = *(DWORD64 *)m_context->Rsp;
			m_context->Rsp += sizeof(DWORD64);
			return m_context;
		}
		RuntimeSupport::VirtualUnwind(UNW_FLAG_NHANDLER, imageBase, m_context->Rip, functionEntry, m_context, m_contextPointers);
		return m_context;
	}

private:
	PCONTEXT m_context;
	PKNONVOLATILE_CONTEXT_POINTERS m_contextPointers;
};
