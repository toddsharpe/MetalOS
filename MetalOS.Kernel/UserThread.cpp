#include "UserThread.h"
#include "x64_support.h"
#include "Main.h"

uint32_t UserThread::LastId = 0;

UserThread::UserThread(ThreadStart startAddress, void* arg, void* stack, UserProcess& process) :
	m_id(++LastId),
	m_process(process),
	m_teb()
{
	process.AddThread(*this);
	
	//Create user thread context
	m_context = new uint8_t[x64_CONTEXT_SIZE];
	x64_init_context(m_context, (void*)stack, process.ThreadCount() == 1 ? process.InitProcess : process.InitThread);

	//Setup args in TEB
	m_teb = (ThreadEnvironmentBlock*)process.HeapAlloc(sizeof(ThreadEnvironmentBlock));
	Assert(m_teb);
	m_teb->SelfPointer = m_teb;
	m_teb->ThreadStart = startAddress;
	m_teb->Arg = arg;
	m_teb->PEB = process.GetPEB();
	m_teb->ThreadId = this->m_id;
}

void UserThread::Display()
{
	Print("UserThread\n");
	Print("     Id: %d\n", m_id);
	Print("  m_teb: 0x%016x\n", m_teb);
	Print("    ctx: 0x%016x\n", m_context);
}

void UserThread::DisplayDetails()
{
	//This reads the TEB which is in the users process space
	Assert(m_process.GetCR3() == __readcr3());
	
	Print("DisplayTEB\n");
	Print("   Self: 0x%016x\n", m_teb->SelfPointer);
	Print("     ID: 0x%016x\n", m_teb->ThreadId);
	Print("    PEB: 0x%016x\n", m_teb->PEB);
	Print("  Start: 0x%016x\n", m_teb->ThreadStart);
	Print("    Arg: 0x%016x\n", m_teb->Arg);

	x64_context* ctx = (x64_context*)m_context;
	Print("  Rsp: 0x%016x Rip: 0x%016x RFlags:0x%08x\n", ctx->Rsp, ctx->Rip, (uint32_t)ctx->Rflags);
}

