#include "KThread.h"
#include "Main.h"

uint32_t KThread::LastId = 0;

KThread::KThread(ThreadStart start, void* arg, void* context, UserThread* userThread) :
	m_id(++LastId),
	m_start(start),
	m_arg(arg),
	m_state(ThreadState::Ready),
	m_waitStatus(),
	m_sleepWake(),
	m_context(context),
	m_userThread(userThread)
{

}

void KThread::Run()
{
	m_start(m_context);
}

void KThread::Display()
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
