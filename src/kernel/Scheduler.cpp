#include "kernel/Scheduler.h"
#include "kernel/Api.h"
#include "Assert.h"
#include "kernel/KThread.h"
#include "Lib/StaticVector.h"
#include "Lib/List.h"

namespace
{
	constexpr size_t MaxKThreads = 32;

	struct CpuContext
	{
		CpuContext() :
			SelfPointer(*this),
			Thread()
		{}

		const CpuContext& SelfPointer;
		KThread* Thread;
	};
	static_assert(offsetof(CpuContext, SelfPointer) == 0, "x64_SYSTEMCALL asm invalid");
	static_assert(offsetof(CpuContext, Thread) == 8, "x64_SYSTEMCALL asm invalid");

	//Per-CPU state. Single context today (uniprocessor); GetThread() already resolves the
	//*current* CPU via GS, so SMP bring-up allocates one CpuContext per CPU (not ruled out here).
	CpuContext m_cpu;

	//Platform
	ReadTsc m_readTsc = nullptr;

	//Threads and current thread. A single shared ready queue (needs a lock under SMP).
	size_t m_threadIndex = 0;
	StaticVector<KThread*, MaxKThreads> m_threads;
}

//Preemption gate.
bool Scheduler::Enabled = false;

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

void Scheduler::Initialize(KProcess& proc, const ReadTsc readTsc)
{
	m_readTsc = readTsc;

	//Make boot thread
	KThread* boot = KeAlloc<KThread>(AllocType::Kernel, nullptr, nullptr, "Boot");
	boot->Process = &proc;
	m_threads.Add(boot);
	boot->Sched.State = KThreadState::Running;

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
		switch (thread->Sched.State)
		{
			//Check timeout
			case KThreadState::Sleeping:
			{
				Assert(thread->Sched.Timeout != 0);
				if (thread->Sched.Timeout <= tsc)
				{
					thread->Sched.Timeout = 0;
					thread->Sched.State = KThreadState::Ready;
					thread->Sched.WaitResult = KWaitResult::None;
				}
			}
			break;

			//Check if timeout has expired or status of signal object
			case KThreadState::SignalWait:
			{
				Assert(thread->Sched.Signal);
				KSignal* signal = thread->Sched.Signal;

				if (thread->Sched.Timeout <= tsc)
				{
					thread->Sched.Timeout = 0;
					thread->Sched.Signal = nullptr;
					thread->Sched.State = KThreadState::Ready;
					thread->Sched.WaitResult = KWaitResult::Timeout;
				}
				else if (signal->IsSignalled())
				{
					thread->Sched.Timeout = 0;
					thread->Sched.Signal = nullptr;
					thread->Sched.State = KThreadState::Ready;
					thread->Sched.WaitResult = KWaitResult::Signaled;

					signal->Observed();
				}
			}
			break;
		}
	}

	//Mark current thread as ready
	if (current->Sched.State == KThreadState::Running)
		current->Sched.State = KThreadState::Ready;

	// Select new thread, round robin
	for (size_t i = 0; i < m_threads.Count(); i++)
	{
		m_threadIndex = (m_threadIndex + 1) % m_threads.Count();
		if (m_threads[m_threadIndex]->Sched.State == KThreadState::Ready)
			break;
	}

	//Mark next thread as running
	KThread& next = *m_threads[m_threadIndex];
	next.Sched.ScheduleTime = tsc * 100;
	Assert(next.Sched.State == KThreadState::Ready);
	next.Sched.State = KThreadState::Running;

	//Add to total cpu time
	if (current->Sched.ScheduleTime)
		current->Sched.TotalCpuTime += (tsc * 100) - current->Sched.ScheduleTime;

	//If both threads are the same short-circuit context switch
	if (next.Id != current->Id)
	{
		if (ArchSaveContext(current->ContextPtr) == 0)
		{
			//Switch cr3 if changing processes
			Assert(next.Process);
			ArchSetPagingRoot(next.Process->Tables.Root);

			//Set current thread
			m_cpu.Thread = &next;

			//Point the user GS base at the incoming thread's TEB; otherwise 'next' resumes
			//with the outgoing thread's TEB as GS and faults reading it in GetPEB.
			if (next.UserThread)
				ArchSetUserGsBase(next.UserThread->TebAddress());

			//Set interrupt stack
			//This is needed since syscall and interrupt paths have different stack lengths
			ArchSetInterruptStack(next.InterruptStack());

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
		if (thread->Sched.State == KThreadState::Terminated)
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
	current.Sched.Timeout = deadline;
	current.Sched.State = KThreadState::Sleeping;

	Schedule();
}

KWaitResult Scheduler::ObjectWait(KSignal& object, const milli_t timeout)
{
	KThread& current = GetCurrentThread();
	AssertEqual(current.Sched.State, KThreadState::Running);
	AssertEqual(current.Sched.Signal, nullptr);

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
	current.Sched.Signal = &object;
	current.Sched.Timeout = deadline;
	current.Sched.State = KThreadState::SignalWait;

	ArchRestoreFlags(flags);
	Schedule();
	return current.Sched.WaitResult;
}

void Scheduler::KillThread(KThread& thread)
{
	const bool isRunning = &thread == &GetCurrentThread();

	thread.Sched.State = KThreadState::Terminated;
	if (!isRunning)
		return;

	Schedule();
	Fatal("Unreachable");
}

void Scheduler::KillProcess(UProcess& process)
{
	const bool isRunning = &process == &GetUProcess();

	for (UThread* const thread : process.Threads)
		thread->Thread.Sched.State = KThreadState::Terminated;

	if (!isRunning)
		return;

	Schedule();
	Fatal("Unreachable");
}

void Scheduler::Display()
{
	Printf("Scheduler::Display\n");
	Printf("    Threads:\n");
	for (size_t i = 0; i < m_threads.Count(); i++)
	{
		KThread* thread = m_threads[i];
		thread->Display();
	}
}
