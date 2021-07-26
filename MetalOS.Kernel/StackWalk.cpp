#include "Kernel.h"
#include "Assert.h"

#include "StackWalk.h"
#include "RuntimeSupport.h"

StackWalk::StackWalk(PCONTEXT context, PKNONVOLATILE_CONTEXT_POINTERS contextPointers) :
	m_context(context),
	m_contextPointers(contextPointers)
{
	Assert(context->Rsp);
	Assert(context->Rbp);
	Assert(context->Rip);
}

bool StackWalk::HasNext()
{
	return m_context->Rip != NULL;
}

PCONTEXT StackWalk::Next(const uintptr_t imageBase)
{
	const PRUNTIME_FUNCTION functionEntry = RuntimeSupport::LookupFunctionEntry(m_context->Rip, imageBase, nullptr);
	if (!functionEntry)
	{
		//Pop IP off the stack
		m_context->Rip = *(DWORD64*)m_context->Rsp;
		m_context->Rsp += sizeof(DWORD64);
		return m_context;
	}

	RuntimeSupport::VirtualUnwind(UNW_FLAG_NHANDLER, imageBase, m_context->Rip, functionEntry, m_context, m_contextPointers);
	return m_context;
}

