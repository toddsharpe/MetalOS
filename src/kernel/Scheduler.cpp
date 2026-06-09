#include "kernel/Scheduler.h"
#include "kernel/Api.h"
#include "Assert.h"
#include "kernel/KThread.h"

KThread& Scheduler::GetThread()
{
	Assert(_readgsbase_u64() != 0);
	CpuContext* ctx = (CpuContext*)__readgsqword(offsetof(CpuContext, SelfPointer));
	Assert(ctx->Thread);
	return *ctx->Thread;
}

UThread& Scheduler::GetUThread()
{
	KThread& current = Scheduler::GetThread();
	Assert(current.UserThread);
	UThread& user = *current.UserThread;
	return user;
}

UProcess& Scheduler::GetUProcess()
{
	UThread& user = Scheduler::GetUThread();
	return user.Process;
}

Scheduler::Scheduler(const ReadTsc readTsc) :
	Enabled(),
	m_cpu(),
	m_readTsc(readTsc),
	m_threadIndex(),
	m_threads()
{

}

void Scheduler::Initialize(KProcess& proc)
{
	//Make boot thread
	KThread* boot = KeAlloc<KThread>(AllocType::Kernel, nullptr, nullptr, "Boot");
	boot->Process = &proc;
	m_threads.Add(boot);
	boot->m_state = KThreadState::Running;

	//Write to CPU state
	ArchSetUserCpuContext(&m_cpu);
	m_cpu.Thread = boot;
}

void Scheduler::MakeReady(KThread& thread)
{
	for (size_t i = 0; i < m_threads.Count(); i++)
		AssertNotEqual(m_threads[i]->Id, thread.Id);

	m_threads.Add(&thread);
}

void Scheduler::Schedule()
{
	const cpu_flags_t flags = ArchDisableInterrupts();
	const nano100_t tsc = m_readTsc();
	KThread* current = m_cpu.Thread;
	Assert(current);

	//Find threads which can be set to ready now
	for (size_t i = 0; i < m_threads.Count(); i++)
	{
		KThread* const thread = m_threads[i];
		switch (thread->m_state)
		{
			//Check timeout
			case KThreadState::Sleeping:
			{
				Assert(thread->m_timeout != 0);
				if (thread->m_timeout <= tsc)
				{
					thread->m_timeout = 0;
					thread->m_state = KThreadState::Ready;
					thread->m_waitResult = KWaitResult::None;
				}
			}
			break;

			//Check if timeout has expired or status of signal object
			case KThreadState::SignalWait:
			{
				Assert(thread->m_signal);
				KSignal* signal = thread->m_signal;

				if (thread->m_timeout <= tsc)
				{
					thread->m_timeout = 0;
					thread->m_signal = nullptr;
					thread->m_state = KThreadState::Ready;
					thread->m_waitResult = KWaitResult::Timeout;
				}
				else if (signal->IsSignalled())
				{
					thread->m_timeout = 0;
					thread->m_signal = nullptr;
					thread->m_state = KThreadState::Ready;
					thread->m_waitResult = KWaitResult::Signaled;

					signal->Observed();
				}
			}
			break;
		}
	}

	//Mark current thread as ready
	if (current->m_state == KThreadState::Running)
		current->m_state = KThreadState::Ready;

	// Select new thread, round robin
	for (size_t i = 0; i < m_threads.Count(); i++)
	{
		m_threadIndex = (m_threadIndex + 1) % m_threads.Count();
		if (m_threads[m_threadIndex]->m_state == KThreadState::Ready)
			break;
	}

	//Mark next thread as running
	KThread& next = *m_threads[m_threadIndex];
	next.m_scheduleTime = tsc * 100;
	Assert(next.m_state == KThreadState::Ready);
	next.m_state = KThreadState::Running;

	//Add to total cpu time
	if (current->m_scheduleTime)
		current->m_totalCpuTime += (tsc * 100) - current->m_scheduleTime;

	//If both threads are the same short-circuit context switch
	if (next.Id != current->Id)
	{
#if 0
		Printf("Scheduler: %d (%s) -> %d (%s)\n", current->Id, current->m_name.c_str(), next.Id, next.m_name.c_str());
		Printf("Old Ctx: 0x%016x (0x%016x), New Ctx: 0x%016x (0x%016x)\n", current->ContextPtr, current->m_context.Rsp, next.ContextPtr, next.m_context.Rsp);
#endif
		if (ArchSaveContext(current->ContextPtr) == 0)
		{
			//Switch cr3 if changing processes
			Assert(next.Process);
			ArchSetPagingRoot(next.Process->Tables.Root);

			//Set current thread
			m_cpu.Thread = &next;

			//Set interrupt stack
			//This is needed since syscall and interrupt paths have different stack lengths
			ArchSetInterruptStack((void*)next.m_stackPointer);

			//Load new context
			ArchLoadContext(next.ContextPtr);
		}
		else
		{
			//Restored thread starts from here
		}
	}

	//On new thread, purge deleted threads
	size_t idx = 0;
	while (idx < m_threads.Count())
	{
		KThread* thread = m_threads[idx];
		if (thread->m_state == KThreadState::Terminated)
		{
			m_threads.RemoveAt(idx);
			KeFree(thread, AllocType::Kernel);
		}
		else
		{
			idx++;
		}
	}
	m_threadIndex = Clamp(m_threadIndex, 0ULL, m_threads.Count() - 1);

	ArchRestoreFlags(flags);
}

//Currently running thread
KThread& Scheduler::GetCurrentThread()
{
	Assert(m_cpu.Thread);
	return *m_cpu.Thread;
}

void Scheduler::Sleep(nano_t value)
{
	KThread& current = GetCurrentThread();

	//Set wakeup
	const nano100_t tscStart = m_readTsc();
	const nano100_t deadline = tscStart + value / 100;
	current.m_timeout = deadline;
	current.m_state = KThreadState::Sleeping;

	this->Schedule();
}

KWaitResult Scheduler::ObjectWait(KSignal& object, const milli_t timeout)
{
	KThread& current = GetCurrentThread();
	AssertEqual(current.m_state, KThreadState::Running);
	AssertEqual(current.m_signal, nullptr);

	//Disable interrupts so the signal check and the SignalWait arm are atomic.
	//Without this, a timer could fire between IsSignalled()=false and
	//m_state=SignalWait, context-switch to the windowing thread, enqueue a
	//message, and Schedule() would not see this thread in SignalWait - missing
	//the wakeup until the next timer tick.
	const cpu_flags_t flags = ArchDisableInterrupts();

	if (object.IsSignalled())
	{
		object.Observed();
		ArchRestoreFlags(flags);
		return KWaitResult::Signaled;
	}

	const nano100_t tscStart = m_readTsc();
	const nano100_t deadline = tscStart + ToNano(timeout) / 100;

	//m_signal and m_timeout must be written before m_state=SignalWait so that
	//any Schedule() seeing SignalWait always finds a valid m_signal.
	current.m_signal = &object;
	current.m_timeout = deadline;
	current.m_state = KThreadState::SignalWait;

	ArchRestoreFlags(flags);
	this->Schedule();
	return current.m_waitResult;
}
void Scheduler::KillThread(KThread& thread)
{
	const bool isRunning = &thread == &GetCurrentThread();

	thread.m_state = KThreadState::Terminated;
	if (!isRunning)
		return;

	this->Schedule();
	Fatal("Unreachable");
}

void Scheduler::KillProcess(UProcess& process)
{
	const bool isRunning = &process == &GetUProcess();
	
	ListForEach<UThread>(process.m_threads, [](const ListEntry&, const UThread& thread)
	{
		KThread& kThread = thread.Thread;
		kThread.m_state = KThreadState::Terminated;
	});

	if (!isRunning)
		return;

	this->Schedule();
	Fatal("Unreachable");
}

void Scheduler::Display() const
{
	Printf("Scheduler::Display\n");
	Printf("    Threads:\n");
	for (size_t i = 0; i < m_threads.Count(); i++)
	{
		KThread* thread = m_threads[i];
		thread->Display();
	}
}
