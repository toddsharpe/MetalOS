#include "Kernel.h"
#include "Assert.h"

#include "KThread.h"
#include <MetalOS.Arch.h>

size_t KThread::LastId = 0;

KThread::KThread(const ThreadStart start, void* arg, UserThread* userThread) :
	m_context(),
	m_userThread(userThread),

	m_start(start),
	m_arg(arg),
	m_id(++LastId),
	m_stackPointer(),
	m_stackAllocation(),

	m_state(ThreadState::Ready),
	m_waitStatus(WaitStatus::None),
	m_sleepWake(),
	m_event(),
	m_suspendCount(),

	Name()
{
	const size_t size = ArchContextSize();
	m_context = new uint8_t[size];
	memset(m_context, 0, size);
}

KThread::~KThread()
{
	Assert(m_context);
	delete m_context;

	//TODO: delete stack if not null

	if (m_userThread != nullptr)
		delete m_userThread;
}

void KThread::Run()
{
	m_start(m_arg);
}

void KThread::InitContext(void* entry)
{
	m_stackAllocation = kernel.AllocateStack(KThread::StackPages);
	m_stackPointer = (void*)((uintptr_t)m_stackAllocation + (KThread::StackPages << PAGE_SHIFT) - ArchStackReserve());
	
	kernel.Printf("KThread::InitContext Id %d Start: 0x%016x, End: 0x%016x\n", m_id, m_stackAllocation, m_stackPointer);
	ArchInitContext(m_context, entry, m_stackPointer);
}

uint32_t KThread::GetId() const
{
	return m_id;
}

//TODO: this returns the starting stack pointer, not where it currently is
void* KThread::GetStackPointer() const
{
	return m_stackPointer;
}

UserThread* KThread::GetUserThread() const
{
	return m_userThread;
}

void KThread::Display() const
{
	kernel.Printf("KThread\n");
	kernel.Printf("     Id: %x\n", m_id);
	kernel.Printf("   Name: %s\n", Name.c_str());
	kernel.Printf("  Start: 0x%016x\n", m_start);
	kernel.Printf("    Arg: 0x%016x\n", m_arg);
	kernel.Printf("  State: %d\n", m_state);
	kernel.Printf("   User: 0x%016x\n", m_userThread);

	switch (m_state)
	{
	case ThreadState::Sleeping:
		kernel.Printf("  SleepAwake: %d\n", m_sleepWake);
		break;
	}

	//TODO: move to arch layer
	x64_context* ctx = (x64_context*)m_context;
	kernel.Printf("  Rbp: 0x%016x Rsp: 0x%016x Rip: 0x%016x RFlags:0x%08x\n", ctx->Rbp, ctx->Rsp, ctx->Rip, (uint32_t)ctx->Rflags);

	if (m_userThread != nullptr)
		m_userThread->Display();
}
