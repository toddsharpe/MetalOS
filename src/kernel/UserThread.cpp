#include "Kernel.h"
#include "Assert.h"
#include <kernel/MetalOS.Arch.h>

#include "UserThread.h"

//ASM interop
extern "C"
{
	constexpr size_t USERTHREAD_STACK_OFF = offsetof(UserThread, Stack);
	static_assert(USERTHREAD_STACK_OFF == 0, "SystemCall.asm invalid");
}

uint32_t UserThread::LastId = 0;

UserThread::UserThread(const ThreadStart startAddress, void* const arg, void* const entry, const size_t stackSize, UserProcess& process) :
	Stack(),
	Deleted(),
	Id(++LastId),
	Process(process),
	m_teb(),
	m_context(),
	m_stackAllocation(),
	m_messages()
{
	const size_t useStackSize = (stackSize == 0) ? UserThread::DefaultStack : stackSize;
	
	//Allocate user stack
	m_stackAllocation = kernel.VirtualAlloc(process, nullptr, useStackSize);
	void* const stackPointer = MakePointer<void*>(m_stackAllocation, PageAlign(useStackSize) - ArchStackReserve());
	
	//Create user thread context
	ArchInitContext(&m_context, entry, stackPointer);

	//Setup args in TEB
	m_teb = process.AllocTEB();
	m_teb->SelfPointer = m_teb;
	m_teb->ThreadStart = startAddress;
	m_teb->Arg = arg;
	m_teb->ThreadId = this->Id;
	m_teb->Handle = this;
}

void UserThread::Run()
{
	ArchUserThreadStart(&m_context, m_teb);
}

void UserThread::EnqueueMessage(const Message& message)
{
	m_messages.push_back(message);
}

bool UserThread::DequeueMessage(Message& message)
{
	if (!HasMessage())
		return false;

	message = m_messages.front();
	m_messages.pop_front();
	return true;
}

bool UserThread::HasMessage() const
{
	return !m_messages.empty();
}

void UserThread::Display()
{
	Printf("UserThread\n");
	Printf("     Id: %d\n", Id);
	Printf("  m_teb: 0x%016x\n", m_teb);
	Printf("Context:\n");
	Printf("  Rsp: 0x%016x \n", m_context.Rsp);
	Printf("  Rip: 0x%016x\n", m_context.Rip);
}

void UserThread::DisplayMessages()
{
	kernel.Printf("DisplayMessages\n");
	for (const Message& msg : m_messages)
	{
		kernel.Printf("  A: 0x%016x Type: %x\n", msg, msg.Header.MessageType);
	}
}

void UserThread::DisplayDetails()
{
	//This reads the TEB which is in the users process space
	Assert(Process.GetCR3() == __readcr3());
	
	kernel.Printf("DisplayTEB\n");
	kernel.Printf("   Self: 0x%016x\n", m_teb->SelfPointer);
	kernel.Printf("     ID: 0x%016x\n", m_teb->ThreadId);
	kernel.Printf("    PEB: 0x%016x\n", m_teb->PEB);
	kernel.Printf("  Start: 0x%016x\n", m_teb->ThreadStart);
	kernel.Printf(" Handle: 0x%016x\n", m_teb->Handle);
	kernel.Printf("    Arg: 0x%016x\n", m_teb->Arg);
}
