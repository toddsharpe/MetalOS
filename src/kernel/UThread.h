#pragma once

#include "kernel/UProcess.h"
#include "user/MetalOS.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/StaticQueue.h"

static uint32_t LastUThreadId = 0;

class KThread;
class UThread
{
public:
	static bool HasMessage(void* const arg) { return ((UThread*)arg)->HasMessage(); };

	UThread(UProcess& process, KThread& thread, ThreadEnvironmentBlock& teb) :
		Link(),
		Stack(),
		Process(process),
		Thread(thread),
		Name(),
		Id(++LastUThreadId),
		m_messages(),
		m_context(),
		m_teb(teb)
	{

	}

	//TODO(tsharpe): Find way to specify this arena in user process space
	void Initialize(void* const stackPointer, void* const threadStart)
	{
		//Create user context
		ArchInitContext(&m_context, threadStart, stackPointer);
	}

	void Start()
	{
		ArchUserThreadStart(&m_context, &m_teb);
	}

	void Enqueue(const Message& message)
	{
		const cpu_flags_t flags = m_lock.Acquire();
		m_messages.Enqueue(message);
		m_lock.Release(flags);
	}

	bool Dequeue(Message& message)
	{
		const cpu_flags_t flags = m_lock.Acquire();
		if (m_messages.IsEmpty())
		{
			m_lock.Release(flags);
			return false;
		}
		message = m_messages.Dequeue();
		m_lock.Release(flags);
		return true;
	}

	bool HasMessage() const
	{
		return !m_messages.IsEmpty();
	}

	void Display() const
	{
		Printf("UThread:\n");
		Printf("     Id: %d\n", Id);
		Printf("   Name: %s\n", Name.c_str());
		Printf("  m_teb: 0x%016x\n", m_teb);
		Printf("  stack: 0x%016x\n", Stack);
		Printf("  Context:\n");
		Printf("   - Rsp: 0x%016x \n", m_context.Rsp);
		Printf("   - Rip: 0x%016x\n", m_context.Rip);
		Process.Display();
	}

	ListEntry Link;

	void* Stack; //Saved from asm on syscalls
	UProcess& Process;
	KThread& Thread;
	CString Name;
	const uint32_t Id;

private:
	//Scheduler

	//Messages
	KSpinLock m_lock;
	StaticQueue<Message, 16> m_messages;

	//Storage
	Context m_context;

	//Pointers into process address space
	ThreadEnvironmentBlock& m_teb;
};
static_assert(offsetof(UThread, Stack) == 16, "x64_SYSTEMCALL asm invalid");
