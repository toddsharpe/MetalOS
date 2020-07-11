#include "Scheduler.h"
#include "Main.h"
#include "x64_support.h"
#include "x64.h"
#include "KSemaphore.h"

Scheduler::Scheduler(KThread& bootThread) :
	Enabled(),
	m_hyperv(),
	m_readyQueue(),
	m_sleepQueue(),
	m_timeouts(),
	m_events()
{
	//Set boot thread as running
	bootThread.m_state = ThreadState::Running;
	
	//Initialize context for one CPU. Big TODO for more CPUs
	CpuContext* context = new CpuContext();
	context->SelfPointer = context;
	context->Thread = &bootThread;

	//Write CPU state
	x64::SetKernelCpuContext(context);
	x64::SetUserCpuContext(context);
}

Scheduler::CpuContext* Scheduler::GetCpuContext() const
{
	Assert(x64_ReadGS() != 0);
	return (CpuContext*)__readgsqword(offsetof(CpuContext, SelfPointer));
}

KThread* Scheduler::GetCurrentThread() const
{
	CpuContext* context = GetCpuContext();
	Assert(context);
	return context->Thread;
}

void Scheduler::SetCurrentThread(KThread& thread)
{
	CpuContext* context = GetCpuContext();
	Assert(context);
	context->Thread = &thread;
}

void Scheduler::Schedule()
{
	if (!Enabled)
		return;

	const uint64_t tsc = m_hyperv.ReadTsc();
	KThread* current = GetCurrentThread();
	
	//Promote off sleep queue
	if (m_sleepQueue.size() > 0)
	{
		auto it = m_sleepQueue.begin();
		while (it != m_sleepQueue.end())
		{
			KThread* item = *it;
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

	if (!m_readyQueue.empty())
	{
		//Save current context
		if (x64_save_context(current->m_context) == 0)
		{
			//Queue current thread if it was preempted
			if (current->m_state == ThreadState::Running)
			{
				current->m_state = ThreadState::Ready;
				m_readyQueue.push_back(current);
			}

			if (current->m_state == ThreadState::Terminated)
			{
				delete current;
			}

			//Get next thread
			KThread* next = m_readyQueue.front();
			Assert(next);
			m_readyQueue.pop_front();

			//Switch cr3 if needed
			UserThread* userThread = next->GetUserThread();
			if (userThread != nullptr)
			{
				const uintptr_t cr3 = userThread->GetProcess().GetCR3();
				if (__readcr3() != cr3)
					__writecr3(cr3);
			}

			//Switch to thread
			next->m_state = ThreadState::Running;
			SetCurrentThread(*next);
			x64_load_context(next->m_context);
		}
	}
}

void Scheduler::AddReady(KThread& thread)
{
	thread.m_state = ThreadState::Ready;
	m_readyQueue.push_back(&thread);
}

void Scheduler::Sleep(nano_t value)
{
	KThread* current = GetCurrentThread();
	
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
				Remove(t);
				delete t;
			}
		}

		delete &process;
	}

	this->Schedule();
}

WaitStatus Scheduler::SemaphoreWait(KSemaphore* semaphore, nano100_t timeout)
{
	KThread* current = GetCurrentThread();

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

void Scheduler::Remove(KThread*& thread)
{
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
		Print("ThreadState: 0x%x\n", thread->m_state);
		Assert(false);
	}
}

void Scheduler::RemoveFromEvent(KThread*& thread)
{
	const auto& it = m_events.find(thread->m_event);
	Assert(it != m_events.end());

	it->second.remove(thread);
}

void Scheduler::Display() const
{
	Print("Scheduler::Schedule - Ready: %d, Sleep: %d, SemTimeout %d, SemWait %d\n", m_readyQueue.size(), m_sleepQueue.size(), m_timeouts.size(),
		m_events.size());
	if (!m_readyQueue.empty())
	{
		Print("  Ready: ");
		for (const auto& i : m_readyQueue)
		{
			Print("%x ", i->GetId());
		}
		Print("\n");
	}
	if (!m_sleepQueue.empty())
	{
		Print("  Sleep: ");
		for (const auto& i : m_sleepQueue)
		{
			Print("%x(0x%016x) ", i->GetId(), i->m_sleepWake);
		}
		Print("\n");
	}
	if (!m_timeouts.empty())
	{
		Print("  Timeouts: ");
		for (const auto& i : m_timeouts)
		{
			Print("%x(0x%016x) ", i->GetId(), i->m_sleepWake);
		}
		Print("\n");
	}
	if (!m_events.empty())
	{
		Print("  Events: ");
		for (const auto& i : m_events)
		{
			KSemaphore* s = (KSemaphore*)i.first;
			Print("0x%016x - ", s);
			for (const auto& j : i.second)
			{
				Print("%x ", j->GetId());
			}
		}
		Print("\n");
	}
}
