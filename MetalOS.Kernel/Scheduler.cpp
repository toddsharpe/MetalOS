#include "Kernel.h"
#include "Assert.h"

#include "Scheduler.h"
#include "KSemaphore.h"
#include <MetalOS.Arch.h>
#include "StackWalk.h"
#include "PortableExecutable.h"

Scheduler::Scheduler(KThread& bootThread) :
	Enabled(),
	m_hyperv(),
	m_readyQueue(),
	m_sleepQueue(),
	m_timeouts(),
	m_events(),
	m_messageWaits()
{
	//Set boot thread as running
	bootThread.m_state = ThreadState::Running;
	
	//Initialize context for one CPU. Big TODO for more CPUs
	CpuContext* context = new CpuContext();
	context->SelfPointer = context;
	context->Thread = &bootThread;

	//Write CPU state
	ArchSetUserCpuContext(context);
}

Scheduler::CpuContext* Scheduler::GetCpuContext()
{
	Assert(_readgsbase_u64() != 0);
	return (CpuContext*)__readgsqword(offsetof(CpuContext, SelfPointer));
}

KThread* Scheduler::GetCurrentThread()
{
	CpuContext* context = GetCpuContext();
	Assert(context);
	return context->Thread;
}

UserThread* Scheduler::GetCurrentUserThread() const
{
	KThread* current = GetCurrentThread();
	Assert(current);
	return current->GetUserThread();
}

UserProcess& Scheduler::GetCurrentProcess() const
{
	KThread* current = GetCurrentThread();
	Assert(current);
	UserThread* user = current->GetUserThread();
	Assert(user);

	return user->GetProcess();
}

void Scheduler::SetCurrentThread(KThread& thread)
{
	CpuContext* context = GetCpuContext();
	Assert(context);
	context->Thread = &thread;
}

void Scheduler::Schedule()
{
	Assert(Enabled);

	const uint64_t tsc = m_hyperv.ReadTsc();
	KThread* current = GetCurrentThread();
	Assert(current);
	
	//Promote off sleep queue
	if (m_sleepQueue.size() > 0)
	{
		auto it = m_sleepQueue.begin();
		while (it != m_sleepQueue.end())
		{
			for (const auto& i : m_sleepQueue)
			{
				Assert(i);
			}
			
			KThread* item = *it;
			Assert(item);
			if (item->m_sleepWake > 0 && item->m_sleepWake <= tsc)
			{
				item->m_state = ThreadState::Ready;
				item->m_sleepWake = 0;
				m_readyQueue.push_back(item);
				it = m_sleepQueue.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	//Promote off timeout queue
	if (m_timeouts.size() > 0)
	{
		auto it = m_timeouts.begin();
		while (it != m_timeouts.end())
		{
			KThread* item = *it;
			Assert(item);
			if (item->m_sleepWake > 0 && item->m_sleepWake <= tsc)
			{
				//Move to ready queue
				item->m_state = ThreadState::Ready;
				item->m_sleepWake = 0;
				item->m_waitStatus = WaitStatus::Timeout;
				m_readyQueue.push_back(item);
				it = m_timeouts.erase(it);

				//Move from event list
				RemoveFromEvent(item);
			}
			else
			{
				it++;
			}
		}
	}

	//Promote off message wait queue
	if (!m_messageWaits.empty())
	{
		auto it = m_messageWaits.begin();
		while (it != m_messageWaits.end())
		{
			KThread* item = *it;
			Assert(item);
			UserThread* user = item->GetUserThread();
			Assert(user);
			if (user->HasMessage())
			{
				//Move to ready queue
				//kernel.Printf("Moving %d to ready, messages: %d\n", item->GetId(), user->m_messages.size());
				item->m_state = ThreadState::Ready;
				m_readyQueue.push_back(item);
				it = m_messageWaits.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	if (!m_readyQueue.empty())
	{
		//Save current context
		if (ArchSaveContext(current->m_context) == 0)
		{
			//kernel.Printf("Current thread\n");
			//current->Display();

			//Queue current thread if it was preempted
			if (current->m_state == ThreadState::Running)
			{
				current->m_state = ThreadState::Ready;
				m_readyQueue.push_back(current);
			}

			if (current->m_state == ThreadState::Terminated)
			{
				kernel.Printf("Deleting Thread\n");
				delete current;
			}

			//Get next thread
			KThread* next = m_readyQueue.front();
			AssertEqual(next->m_state, ThreadState::Ready);
			m_readyQueue.remove(next);

			while (next->IsSuspended())
			{
				kernel.Printf("-Suspended %x\n", next->GetId());
				m_readyQueue.push_back(next);

				next = m_readyQueue.front();
				AssertEqual(next->m_state, ThreadState::Ready);
				m_readyQueue.remove(next);
			}

			//Switch cr3 if needed
			UserThread* userThread = next->GetUserThread();
			if (userThread != nullptr)
			{
				const uintptr_t cr3 = userThread->GetProcess().GetCR3();
				ArchSetPagingRoot(cr3);
			}

			//kernel.Printf("Next thread\n");
			//next->Display();

			//Switch to thread
			next->m_state = ThreadState::Running;
			SetCurrentThread(*next);
			//TODO: this isnt the current stack pointer of the context struct
			ArchSetInterruptStack(next->GetStackPointer());
			ArchLoadContext(next->m_context);
		}
	}
}

void Scheduler::AddReady(KThread& thread)
{
	kernel.Printf("AddReady %x\n", thread.GetId());
	thread.m_state = ThreadState::Ready;
	m_readyQueue.push_back(&thread);
}

void Scheduler::Sleep(nano_t value)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	//kernel.Printf("Sleep\n");
	
	const nano100_t value100 = value / 100;
	const nano100_t tscStart = m_hyperv.ReadTsc();
	current->m_sleepWake = tscStart + value100;
	current->m_state = ThreadState::Sleeping;
	m_sleepQueue.push_back(current);

	this->Schedule();
}

void Scheduler::KillThread()
{
	KThread* current = GetCurrentThread();
	Assert(current);
	kernel.Printf("KillThread %x\n", current->GetId());
	
	//Mark thread deleted
	current->m_state = ThreadState::Terminated;

	UserThread* user = current->GetUserThread();
	if (user)
	{
		UserProcess& process = user->GetProcess();

		//Check if process is being deleted
		if (process.Delete)
		{
			for (auto& t : process.m_threads)
			{
				if (t->GetId() == current->GetId())
					continue;
				
				Remove(t);
				delete t->GetUserThread();
				delete t;
			}
		}

		delete &process;
	}

	this->Schedule();
}

//TODO: If more than 1 processor, reschedule to suspend
size_t Scheduler::Suspend(KThread& thread)
{
	kernel.Printf("Suspend %x\n", thread.GetId());
	
	const size_t ret = thread.m_suspendCount;
	
	thread.m_suspendCount++;
	return ret;
}
//What does this even do?
size_t Scheduler::Resume(KThread& thread)
{
	kernel.Printf("Resume %x\n", thread.GetId());
	
	const size_t ret = thread.m_suspendCount;
	
	if (thread.m_suspendCount == 0)
		return ret;

	thread.m_suspendCount--;
	return ret;
}

WaitStatus Scheduler::SemaphoreWait(KSemaphore* semaphore, nano100_t timeout)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	//If there is no contention return immediately
	bool result = semaphore->TryWait(1);
	if (result)
		return WaitStatus::Signaled;

	//Add to event wait queue
	current->m_event = semaphore;
	m_events[semaphore].push_back(current);

	//Add timeout
	const nano100_t tscStart = m_hyperv.ReadTsc();
	m_timeouts.push_back(current);
	current->m_state = ThreadState::Waiting;
	current->m_sleepWake = tscStart + timeout;

	//Context switch
	this->Schedule();

	//Return result of wait
	return current->m_waitStatus;
}

void Scheduler::SemaphoreRelease(KSemaphore* semaphore, size_t count)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	int64_t previous = semaphore->Signal(count);
	const size_t scheduleCount = std::clamp(-1 * previous, 0LL, (int64_t)count);

	for (size_t i = 0; i < scheduleCount; i++)
	{
		if (m_events[semaphore].empty())
			break;

		KThread* thread = m_events[semaphore].front();
		Assert(thread);
		m_events[semaphore].pop_front();

		thread->m_state = ThreadState::Ready;
		thread->m_sleepWake = 0;
		thread->m_waitStatus = WaitStatus::Signaled;
		m_readyQueue.push_back(thread);

		//Remove from timeout
		m_timeouts.remove(thread);
	}
}

//If there is a message, return immediately
Message* Scheduler::MessageWait()
{
	KThread* current = GetCurrentThread();
	Assert(current);
	UserThread* user = current->GetUserThread();
	Assert(user);

	//If there is a message, return immediately
	Message* msg = user->DequeueMessage();
	if (msg)
		return msg;

	//Block
	current->m_state = ThreadState::MessageWait;
	m_messageWaits.push_back(current);

	//Context switch
	this->Schedule();

	//Return message
	msg = user->DequeueMessage();
	Assert(msg);
	return msg;
}

void Scheduler::Remove(KThread*& thread)
{
	kernel.Printf("Remove %x\n", thread->GetId());
	switch (thread->m_state)
	{
	case ThreadState::Ready:
		m_readyQueue.remove(thread);
		break;

	case ThreadState::Sleeping:
		m_sleepQueue.remove(thread);
		break;

	case ThreadState::Waiting:
		m_timeouts.remove(thread);
		RemoveFromEvent(thread);
		break;

	case ThreadState::Terminated:
		//Do nothing
		break;

	default:
		kernel.Printf("ThreadState: 0x%x\n", thread->m_state);
		Assert(false);
	}
}

void Scheduler::RemoveFromEvent(KThread*& thread)
{
	kernel.Printf("RemoveFromEvent %x\n", thread->GetId());
	
	const auto& it = m_events.find(thread->m_event);
	Assert(it != m_events.end());

	it->second.remove(thread);
}

void Scheduler::Display() const
{
	kernel.Printf("Scheduler::Schedule - Ready: %d, Sleep: %d, SemTimeout %d, SemWait %d, MsgWait %d\n", m_readyQueue.size(), m_sleepQueue.size(), m_timeouts.size(),
		m_events.size(), m_messageWaits.size());
	if (!m_readyQueue.empty())
	{
		kernel.Printf("  Ready: ");
		for (const auto& i : m_readyQueue)
		{
			kernel.Printf("i: 0x%016x - ", i);
			kernel.Printf("%x ", i->GetId());
		}
		kernel.Printf("\n");
	}
	if (!m_sleepQueue.empty())
	{
		kernel.Printf("  Sleep: ");
		for (const auto& i : m_sleepQueue)
		{
			kernel.Printf("i: 0x%016x - ", i);
			if (i != nullptr)
				kernel.Printf("%x(0x%016x)\n", i->GetId(), i->m_sleepWake);
			else
				kernel.Printf("null\n");
		}
		kernel.Printf("\n");
	}
	if (!m_timeouts.empty())
	{
		kernel.Printf("  Timeouts: ");
		for (const auto& i : m_timeouts)
		{
			kernel.Printf("%x(0x%016x) ", i->GetId(), i->m_sleepWake);
		}
		kernel.Printf("\n");
	}
	if (!m_events.empty())
	{
		kernel.Printf("  Events: ");
		for (const auto& i : m_events)
		{
			KSemaphore* s = (KSemaphore*)i.first;
			kernel.Printf("0x%016x - ", s);
			for (const auto& j : i.second)
			{
				kernel.Printf("%x ", j->GetId());
			}
		}
		kernel.Printf("\n");
	}
	if (!m_messageWaits.empty())
	{
		kernel.Printf("  MsgWaits: ");
		for (const auto& i : m_messageWaits)
		{
			kernel.Printf("i: 0x%016x\n", i);
			if (i != nullptr)
				kernel.Printf("%x ", i->GetId());
			else
				kernel.Printf("null\n");
		}
		kernel.Printf("\n");
	}
}
