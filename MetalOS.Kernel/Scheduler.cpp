#include "Scheduler.h"
#include "Main.h"
#include "x64_support.h"
#include "x64.h"
#include "KSemaphore.h"

Scheduler::Scheduler(KThread& bootThread) :
	m_hyperv(),
	Enabled(),
	m_readyQueue(),
	m_sleepQueue(),
	m_semaphoreTimeouts(),
	m_semaphoreWaits()
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

Scheduler::CpuContext* Scheduler::GetCpuContext()
{
	Assert(x64_ReadGS() != 0);
	return (CpuContext*)__readgsqword(offsetof(CpuContext, SelfPointer));
}

KThread* Scheduler::GetCurrentThread()
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
	
	//Display();

	//Promote off sleep queue
	if (m_sleepQueue.size() > 0)
	{
		auto it = m_sleepQueue.begin();
		while (it != m_sleepQueue.end())
		{
			const uint32_t id = *it;
			KThread* item = kernel.GetKernelThread(id);
			//Print("  %d = 0x%x\n", id, item->SleepWake);
			if (item->m_sleepWake > 0 && item->m_sleepWake <= tsc)
			{
				item->m_state = ThreadState::Ready;
				item->m_sleepWake = 0;
				m_readyQueue.push_back(id);
				//Print("  Awake!\n");

				it = m_sleepQueue.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	//Promote off semaphore timeout queue
	if (m_semaphoreTimeouts.size() > 0)
	{
		auto it = m_semaphoreTimeouts.begin();
		while (it != m_semaphoreTimeouts.end())
		{
			const uint32_t id = *it;
			KThread* item = kernel.GetKernelThread(id);
			//Print("  %d = 0x%x\n", id, item->SleepWake);
			if (item->m_sleepWake > 0 && item->m_sleepWake <= tsc)
			{
				item->m_state = ThreadState::Ready;
				item->m_sleepWake = 0;
				item->m_waitStatus = WaitStatus::Timeout;
				m_readyQueue.push_back(id);
				//Print("  Awake!\n");
				
				Assert(false);//This doesn't remove it from wait queue!!!!

				it = m_semaphoreTimeouts.erase(it);
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
				m_readyQueue.push_back(current->GetId());
			}

			//Get next thread
			uint32_t nextId = m_readyQueue.front();
			m_readyQueue.pop_front();
			KThread* next = kernel.GetKernelThread(nextId);
			Assert(next);

			//TODO: change cr3!!!

			//Print("  Schedule: 0x%x -> 0x%x\n", current->GetId(), next->GetId());

			//Switch to thread
			next->m_state = ThreadState::Running;
			SetCurrentThread(*next);
			x64_load_context(next->m_context);//Does not return
		}
	}
}

void Scheduler::Sleep(nano_t value)
{
	//Print("Scheduler::Sleep\n");
	KThread* current = GetCurrentThread();
	
	const nano100_t value100 = value / 100;
	const nano100_t tscStart = m_hyperv.ReadTsc();
	current->m_sleepWake = tscStart + value100;
	current->m_state = ThreadState::Waiting;
	m_sleepQueue.push_back(current->GetId());

	//Print("  Wake: 0x%016x\n", current->SleepWake);

	this->Schedule();
}

WaitStatus Scheduler::SemaphoreWait(KSemaphore* semaphore, nano100_t timeout)
{
	KThread* current = GetCurrentThread();
	//Print("Scheduler::SemaphoreWait: %s - %d\n", semaphore->GetName(), current->Id);

	//If there is no contention return immediately
	bool result = semaphore->TryWait(1);
	if (result)
		return WaitStatus::Signaled;

	//Add to semaphore wait queue
	m_semaphoreWaits[semaphore].push_back(current->GetId());

	//Add timeout
	const nano100_t tscStart = m_hyperv.ReadTsc();
	m_semaphoreTimeouts.push_back(current->GetId());
	current->m_state = ThreadState::Waiting;
	current->m_sleepWake = tscStart + timeout;

	this->Schedule();
	//Print("  Waited: %d - %d\n", current->Id, current->WaitStatus);

	return current->m_waitStatus;
}

void Scheduler::SemaphoreRelease(KSemaphore* semaphore, size_t count)
{
	KThread* current = GetCurrentThread();
	//Print("SemaphoreRelease: %s - %d\n", semaphore->GetName(), current->Id);

	int64_t previous = semaphore->Signal(count);
	//Print("  Previous: %d Now: %d\n", previous, semaphore->Value());

	const size_t scheduleCount = std::clamp(-1 * previous, 0LL, (int64_t)count);
	//Print("  scheduleCount: %d\n", scheduleCount);

	for (size_t i = 0; i < scheduleCount; i++)
	{
		if (m_semaphoreWaits[semaphore].empty())
			break;

		const uint32_t id = m_semaphoreWaits[semaphore].front();
		//Print("  Id: %d\n", id);
		m_semaphoreWaits[semaphore].pop_front();

		KThread* item = kernel.GetKernelThread(id);
		item->m_state = ThreadState::Ready;
		item->m_sleepWake = 0;
		item->m_waitStatus = WaitStatus::Signaled;
		m_readyQueue.push_back(id);

		//Remove from timeout
		m_semaphoreTimeouts.remove(id);
	}
}

void Scheduler::Add(KThread& thread)
{
	thread.Display();

	switch (thread.m_state)
	{
	case ThreadState::Ready:
		m_readyQueue.push_back(thread.GetId());
		break;
	default:
		Assert(false);
	}
}

void Scheduler::Display()
{
	Print("Scheduler::Schedule - Ready: %d, Sleep: %d, SemTimeout %d, SemWait %d\n", m_readyQueue.size(), m_sleepQueue.size(), m_semaphoreTimeouts.size(),
		m_semaphoreWaits.size());
	if (!m_readyQueue.empty())
	{
		Print("  Ready: ");
		for (const auto& i : m_readyQueue)
		{
			Print("%x ", i);
		}
		Print("\n");
	}
	if (!m_sleepQueue.empty())
	{
		Print("  Sleep: ");
		for (const auto& i : m_sleepQueue)
		{
			KThread* t = kernel.GetKernelThread(i);
			Print("%x(0x%016x) ", i, t->m_sleepWake);
		}
		Print("\n");
	}
	if (!m_semaphoreTimeouts.empty())
	{
		Print("  SemaphoreTimeouts: ");
		for (const auto& i : m_semaphoreTimeouts)
		{
			KThread* t = kernel.GetKernelThread(i);
			Print("%x(0x%016x) ", i, t->m_sleepWake);
		}
		Print("\n");
	}
	if (!m_semaphoreWaits.empty())
	{
		Print("  SemaphoreWaits: ");
		for (const auto& i : m_semaphoreWaits)
		{
			Print("%s - ", i.first->GetName());
			for (const auto& j : i.second)
			{
				Print("%x ", j);
			}
		}
		Print("\n");
	}
}
