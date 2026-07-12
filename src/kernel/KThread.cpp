#include "kernel/KThread.h"
#include "kernel/Api.h"
#include "kernel/Arch.h"

uint32_t KThread::LastId = 0;

KThread::KThread(const KThreadStart start, void* const arg, const CString& name) :

	ContextPtr(&m_context),
	UserThread(),

	Id(++LastId),

	Sched(),

	m_start(start),
	m_arg(arg),
	m_stack(),
	m_stackPointer(),

	m_context(),
	m_name(name)
{

}

void KThread::Init(void* const entry)
{
	m_stack = KeVirtualAlloc(KThread::StackSize);
	m_stackPointer = MakePointer<void*>(m_stack, KThread::StackSize - ArchStackReserve());

	Printf("KThread::InitContext Id %d Start: 0x%016x, End: 0x%016x\n", Id, m_stack, m_stackPointer);
	ArchInitContext(ContextPtr, entry, m_stackPointer);
}

void KThread::Start()
{
	m_start(m_arg);
}

void KThread::Display() const
{
	Printf("KThread\n");
	Printf("     Id: %x\n", Id);
	Printf("   Name: %s\n", m_name.c_str());
	Printf("  Start: 0x%016x\n", m_start);
	Printf("    Arg: 0x%016x\n", m_arg);
	Printf("  State: %d\n", Sched.State);
	Printf("WResult: %d\n", Sched.WaitResult);
	Printf("Timeout: %u\n", Sched.Timeout);
	Printf("   User: 0x%016x\n", UserThread);

	if (Sched.Signal)
		Sched.Signal->Display();

	//Display context
	Printf("  Rbp: 0x%016x Rsp: 0x%016x Rip: 0x%016x\n", m_context.Rbp, m_context.Rsp, m_context.Rip);

	if (UserThread != nullptr)
		UserThread->Display();
}
