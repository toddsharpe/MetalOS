#include "Kernel.h"
#include "Assert.h"
#include <MetalOS.Arch.h>

#include "UserThread.h"

uint32_t UserThread::LastId = 0;

UserThread::UserThread(ThreadStart startAddress, void* arg, void* entry, size_t stackSize, UserProcess& process) :
	m_id(++LastId),
	m_process(process),
	m_teb(),
	m_stackAllocation(),
	m_context(),
	m_messages()
{
	if (stackSize == 0)
		stackSize = UserThread::DefaultStack;
	
	//Allocate user stack
	m_stackAllocation = kernel.VirtualAlloc(process, nullptr, stackSize, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWrite);
	void* stackPointer = (void*)((uintptr_t)m_stackAllocation + PageAlign(stackSize) - ArchStackReserve());
	
	//Create user thread context
	const size_t size = ArchContextSize();
	m_context = new uint8_t[size];
	memset(m_context, 0, size);
	ArchInitContext(m_context, entry, stackPointer);

	//Setup args in TEB
	m_teb = process.AllocTEB();
	m_teb->SelfPointer = m_teb;
	m_teb->ThreadStart = startAddress;
	m_teb->Arg = arg;
	m_teb->ThreadId = this->m_id;
	m_teb->Handle = this;
}

void UserThread::Run()
{
	ArchUserThreadStart(m_context, m_teb);
}

Message* UserThread::DequeueMessage()
{
	if (!HasMessage())
		return nullptr;

	Message* top = m_messages.front();
	m_messages.pop_front();
	return top;
}

void UserThread::EnqueueMessage(Message* message)
{
	Assert(message);
	m_messages.push_back(message);
}

void UserThread::Display()
{
	kernel.Printf("UserThread\n");
	kernel.Printf("     Id: %d\n", m_id);
	kernel.Printf("  m_teb: 0x%016x\n", m_teb);
	kernel.Printf("    ctx: 0x%016x\n", m_context);

	x64_context* ctx = (x64_context*)m_context;
	kernel.Printf("  Rsp: 0x%016x Rip: 0x%016x RFlags:0x%08x\n", ctx->Rsp, ctx->Rip, (uint32_t)ctx->Rflags);
}

void UserThread::DisplayMessages()
{
	kernel.Printf("DisplayMessages\n");
	for (const auto& msg : m_messages)
	{
		kernel.Printf("A: 0x%016x Type: %x\n", msg, msg->Header.MessageType);
	}
}

void UserThread::DisplayDetails()
{
	//This reads the TEB which is in the users process space
	Assert(m_process.GetCR3() == __readcr3());
	
	kernel.Printf("DisplayTEB\n");
	kernel.Printf("   Self: 0x%016x\n", m_teb->SelfPointer);
	kernel.Printf("     ID: 0x%016x\n", m_teb->ThreadId);
	kernel.Printf("    PEB: 0x%016x\n", m_teb->PEB);
	kernel.Printf("  Start: 0x%016x\n", m_teb->ThreadStart);
	kernel.Printf(" Handle: 0x%016x\n", m_teb->Handle);
	kernel.Printf("    Arg: 0x%016x\n", m_teb->Arg);
}

