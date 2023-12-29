#include "Scheduler.h"

#include "Assert.h"
#include "Kernel/Objects/KSemaphore.h"
#include "StackWalk.h"
#include "PortableExecutable.h"
#include "MetalOS.Arch.h"
#include "Kernel/Objects/UPipe.h"

KThread* Scheduler::GetThread()
{
	Assert(_readgsbase_u64() != 0);
	CpuContext* ctx = (CpuContext*)__readgsqword(offsetof(CpuContext, SelfPointer));
	Assert(ctx->Thread);
	return ctx->Thread;
}

Scheduler::Scheduler() :
	Enabled(),
	m_hyperv(),
	m_cpu(),
	m_threadIndex(),
	m_threads()
{

}

void Scheduler::Init()
{
	//Make boot thread
	std::shared_ptr<KThread> boot = std::make_shared<KThread>(nullptr, nullptr);
	boot->Name = "Boot";
	boot->m_state = ThreadState::Running;
	m_threads.push_back(boot);

	//Write to CPU state
	ArchSetUserCpuContext(&m_cpu);
}

void Scheduler::Schedule()
{
	Assert(Enabled);

	const uint64_t tsc = m_hyperv.ReadTsc();
	KThread& current = GetCurrentThread();

	//Purge deleted threads if its not the executing one
	auto it = m_threads.begin();
	while (it != m_threads.end())
	{
		Assert(it->get());
		KThread& thread = *it->get();

		if ((thread.m_state == ThreadState::Terminated) && (thread.Id != current.Id))
		{
			if (thread.UserThread != nullptr)
				Assert(thread.UserThread->Deleted);

			it = m_threads.erase(it);
		}
		else
		{
			it++;
		}
	}

	//Iterate through threads and update their status
	for (std::shared_ptr<KThread>& item : m_threads)
	{
		Assert(item.get());
		KThread& thread = *item.get();

		switch (thread.m_state)
		{
			//Check timeout
			case ThreadState::Sleeping:
			{
				Assert(thread.m_timeout != 0);
				if (thread.m_timeout <= tsc)
				{
					thread.m_timeout = 0;
					thread.m_state = ThreadState::Ready;
					thread.m_waitStatus = WaitStatus::None;
				}
			}
			break;

			//Check if timeout has expired or status of signal object
			case ThreadState::SignalWait:
			{
				Assert(thread.m_signal);
				KSignalObject* signal = thread.m_signal;

				if (thread.m_timeout <= tsc)
				{
					thread.m_timeout = 0;
					thread.m_state = ThreadState::Ready;
					thread.m_waitStatus = WaitStatus::Timeout;
				}
				else if (signal->IsSignalled())
				{
					thread.m_timeout = 0;
					thread.m_signal = nullptr;
					thread.m_state = ThreadState::Ready;
					thread.m_waitStatus = WaitStatus::Signaled;

					signal->Observed();
				}
			}
			break;
		}
	}

	//Mark current thread as ready
	if (current.m_state == ThreadState::Running)
		current.m_state = ThreadState::Ready;

	// Select new thread, round robin
	while (true)
	{
		m_threadIndex = (m_threadIndex + 1) % m_threads.size();

		if (m_threads[m_threadIndex]->m_state == ThreadState::Ready)
			break;
	}

	//Mark next thread as running
	KThread& next = *m_threads[m_threadIndex].get();
	next.m_state = ThreadState::Running;

	//If both threads are the same short-circuit context switch
	if (next.Id == current.Id)
		return;

#if FALSE
	Printf("Scheduler: %d (%s) -> %d (%s)\n", current.Id, current.Name.c_str(), next.Id, next.Name.c_str());

	//Save current context
	Printf("Old Ctx: 0x%016x (0x%016x), New Ctx: 0x%016x (0x%016x)\n", current.Context, current.Context->Rsp, next.Context, next.Context->Rsp);
	if (current.UserThread)
		Printf("    Old User Stack: 0x%016x\n", current.UserThread->Stack);
	if (next.UserThread)
		Printf("    New User Stack: 0x%016x\n", next.UserThread->Stack);
#endif

	if (ArchSaveContext(current.Context) == 0)
	{
		//Switch cr3 if changing processes
		UserThread* userThread = next.UserThread;
		if (userThread != nullptr)
		{
			const uintptr_t cr3 = userThread->Process.GetCR3();
			ArchSetPagingRoot(cr3);
		}

		//Set current thread
		m_cpu.Thread = &next;

		//Set interrupt stack
		//TODO(tsharpe): Syscall and interrupt handlers have different stack depths. RSP here is effectively reset,
		//determine if this is right.
		//ArchSetInterruptStack((void*)next.Context->Rsp);
		ArchSetInterruptStack((void*)next.m_stackPointer);

		//Load new context
		ArchLoadContext(next.Context);
	}
}

KThread& Scheduler::GetCurrentThread()
{
	AssertOp(m_threadIndex, <, m_threads.size());
	KThread& thread = *m_threads[m_threadIndex].get();
	return thread;
}

UserThread& Scheduler::GetCurrentUserThread()
{
	KThread& current = GetCurrentThread();
	Assert(current.UserThread);
	return *current.UserThread;
}

UserProcess& Scheduler::GetCurrentProcess()
{
	UserThread& user = GetCurrentUserThread();

	return user.Process;
}

void Scheduler::AddReady(std::shared_ptr<KThread>& thread)
{
	//Mark thread ready
	thread.get()->m_state = ThreadState::Ready;

	//Sanity check this thread isn't already in the ready queue
	for (size_t i = 0; i < m_threads.size(); i++)
	{
		AssertNotEqual(m_threads[i]->Id, thread->Id);
	}

	//Add to threads
	m_threads.push_back(thread);
}

void Scheduler::Sleep(nano_t value)
{
	KThread& current = GetCurrentThread();

	//Set wakeup
	const nano100_t tscStart = m_hyperv.ReadTsc();
	const nano100_t deadline = tscStart + value / 100;
	current.m_timeout = deadline;
	current.m_state = ThreadState::Sleeping;

	//Context switch
	this->Schedule();
}

void Scheduler::KillThread(KThread& thread)
{
	Printf("KillThread %x\n", thread.Id);

	//Mark thread deleted
	thread.m_state = ThreadState::Terminated;

	//Mark user thread deleted
	UserThread* user = thread.UserThread;
	if (user)
	{
		user->Deleted = true;
	}
}

void Scheduler::KillCurrentThread()
{
	KThread& current = GetCurrentThread();

	this->KillThread(current);
	Assert(current.m_state == ThreadState::Terminated);

	this->Schedule();
	Fatal("Unreachable");
}

void Scheduler::KillCurrentProcess()
{
	UserProcess& process = GetCurrentProcess();

	//Kill threads
	for (auto& thread : process.m_threads)
	{
		this->KillThread(*thread);
	}

	//Handle pipe reader/writer counts
	for (auto& item : process.m_objects)
	{
		std::shared_ptr<UObject> uObject = item.second;
		
		if (uObject->Type == UObjectType::Pipe)
		{
			UPipe* uPipe = (UPipe*)uObject.get();
			if (uPipe->Op == PipeOp::Read)
			{
				uPipe->Pipe->Readers--;
			}
			else
			{
				Assert(uPipe->Op == PipeOp::Write);
				uPipe->Pipe->Writers--;
			}
		}
	}

	//Clear up these objects
	process.m_objects.clear();

	//TODO: clean up process?

	//Signal process
	process.m_state = ProcessState::Terminated;

	this->Schedule();
	Fatal("Unreachable");
}

WaitStatus Scheduler::ObjectWait(KSignalObject& object, const milli_t timeout)
{
	KThread& current = GetCurrentThread();
	AssertEqual(current.m_state, ThreadState::Running);
	AssertEqual(current.m_signal, nullptr);

	//Don't block if object is signalled
	if (object.IsSignalled())
	{
		object.Observed();
		return WaitStatus::Signaled;
	}

	//Calculate deadline
	const nano100_t tscStart = m_hyperv.ReadTsc();
	const nano100_t deadline = tscStart + ToNano(timeout) / 100;

	//Set signal
	current.m_state = ThreadState::SignalWait;
	current.m_signal = &object;
	current.m_timeout = deadline;

	this->Schedule();
	return current.m_waitStatus;
}

/*
WaitStatus Scheduler::SemaphoreWait(KSemaphore& semaphore, nano100_t timeout)
{
	const bool wasSignaled = semaphore.IsSignalled();
	semaphore.Wait();

	if (wasSignaled)
		return WaitStatus::Signaled;

	return this->ObjectWait(semaphore, timeout * 100);
}
*/

void Scheduler::Display() const
{
	Printf("Scheduler::Display\n");
	Printf("    Threads: %d\n", m_threads.size());

	for (size_t i = 0; i < m_threads.size(); i++)
	{
		m_threads[i]->Display();
	}
}
