#include "KThread.h"
#include "Main.h"
#include <MetalOS.Arch.h>

uint32_t KThread::LastId = 0;

KThread::KThread(ThreadStart start, void* arg, UserThread* userThread) :
	m_id(++LastId),
	m_start(start),
	m_arg(arg),
	m_context(),
	m_stackPointer(),
	m_stackAllocation(),
	m_state(ThreadState::Ready),
	m_waitStatus(WaitStatus::None),
	m_sleepWake(),
	m_event(),
	m_suspendCount(),
	m_userThread(userThread)
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
	m_stackAllocation = kernel.AllocateKernelPage(0, KThread::StackPages, MemoryProtection::PageReadWrite);
	m_stackPointer = (void*)((uintptr_t)m_stackAllocation + (KThread::StackPages << PAGE_SHIFT) - ArchStackReserve());
	
	ArchInitContext(m_context, entry, m_stackPointer);
}

uint32_t KThread::GetId() const
{
	return m_id;
}

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
	Print("KThread\n");
	Print("     Id: %d\n", m_id);
	Print("  Start: 0x%016x\n", m_start);
	Print("    Arg: 0x%016x\n", m_arg);
	Print("  State: %d\n", m_state);
	Print("   User: 0x%016x\n", m_userThread);

	x64_context* ctx = (x64_context*)m_context;
	Print("  Rbp: 0x%016x Rsp: 0x%016x Rip: 0x%016x RFlags:0x%08x\n", ctx->Rbp, ctx->Rsp, ctx->Rip, (uint32_t)ctx->Rflags);

	if (m_userThread != nullptr)
		m_userThread->Display();
}
