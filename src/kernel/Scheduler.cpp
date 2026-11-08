#include "kernel/Scheduler.h"
#include "kernel/Heap.h"
#include "kernel/Process.h"
#include "Assert.h"
#include "kernel/Core/KThread.h"
#include "Lib/StaticVector.h"

namespace
{
	static constexpr size_t MaxKThreads = 128;

	struct CpuContext
	{
		CpuContext() :
			SelfPointer(*this),
			Thread(),
			UserStack()
		{}

		const CpuContext& SelfPointer;
		KThread* Thread;
		uint64_t UserStack; //Scratch: user RSP stashed by x64_SYSTEMCALL before the kernel stack is loaded
	};
	static_assert(offsetof(CpuContext, SelfPointer) == 0, "x64_SYSTEMCALL asm invalid");
	static_assert(offsetof(CpuContext, Thread) == 8, "x64_SYSTEMCALL asm invalid");
	static_assert(offsetof(CpuContext, UserStack) == 16, "x64_SYSTEMCALL asm invalid");

	//Per-CPU state, which x64_SYSTEMCALL reaches through the kernel GS base. Single context today (uniprocessor).
	CpuContext m_cpu;

	//Platform clock, so the scheduler doesn't depend on Hyper-V
	ReadTsc m_readTsc = nullptr;

	//Threads and current thread. A single shared ready queue (needs a lock under SMP).
	size_t m_threadIndex = 0;
	StaticVector<KThread*, MaxKThreads> m_threads;
	KThread* m_idleThread = nullptr;

	//Terminated threads, freed by the reaper once nothing can be running on their stacks
	KThread* m_deadThreads = nullptr;
}

//Preemption gate.
static bool Enabled = false;

void Scheduler::Enable()
{
	Enabled = true;
}

void Scheduler::Disable()
{
	Enabled = false;
}

bool Scheduler::IsEnabled()
{
	return Enabled;
}

KThread& Scheduler::GetCurrentThread()
{
	Assert(m_cpu.Thread);
	return *m_cpu.Thread;
}

UThread& Scheduler::GetUThread()
{
	KThread& current = Scheduler::GetCurrentThread();
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

bool Scheduler::HasRoom()
{
	return m_threads.Count() < MaxKThreads;
}

void Scheduler::MakeReady(KThread& thread)
{
	for (size_t i = 0; i < m_threads.Count(); i++)
		AssertNotEqual(m_threads[i]->Id, thread.Id);

	Assert(m_threads.Add(&thread));
}

void Scheduler::SetIdleThread(KThread& thread)
{
	m_idleThread = &thread;
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

	// Select new thread, round robin, falling back to idle only when nothing else is ready
	KThread* selected = m_idleThread;
	for (size_t i = 0; i < m_threads.Count(); i++)
	{
		m_threadIndex = (m_threadIndex + 1) % m_threads.Count();
		KThread* const candidate = m_threads[m_threadIndex];
		if (candidate != m_idleThread && candidate->Sched.State == KThreadState::Ready)
		{
			selected = candidate;
			break;
		}
	}

	//Mark next thread as running
	Assert(selected);
	KThread& next = *selected;
	Assert(next.Sched.State == KThreadState::Ready);
	next.Sched.State = KThreadState::Running;

	//If both threads are the same short-circuit context switch
	if (next.Id != current->Id)
	{
		if (ArchSaveContext(current->ContextPtr) == 0)
		{
			//Only user threads carry FPU/SSE state; nothing in the kernel touches it until next resumes
			if (current->UserThread)
				current->UserThread->SaveFpu();
			if (next.UserThread)
				next.UserThread->LoadFpu();

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
			//The UThread outlives its KThread (process thread list, thread handles)
			if (thread->UserThread)
				thread->UserThread->Thread = nullptr;

			//Freeing here would mean heap work in interrupt context, so the reaper does it
			m_threads.RemoveAt(idx);
			thread->NextDead = m_deadThreads;
			m_deadThreads = thread;
			KeSignalReaper();
		}
		else
		{
			idx++;
		}
	}
	m_threadIndex = Clamp(m_threadIndex, 0ULL, m_threads.Count() - 1);

	ArchRestoreFlags(flags);
}

KThread* Scheduler::TakeDeadThread()
{
	const cpu_flags_t flags = ArchDisableInterrupts();
	KThread* const thread = m_deadThreads;
	if (thread)
		m_deadThreads = thread->NextDead;
	ArchRestoreFlags(flags);
	return thread;
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

	//Disable interrupts so the signal check and the SignalWait arm are atomic, else a wakeup between them is missed until the next tick
	const cpu_flags_t flags = ArchDisableInterrupts();

	if (object.IsSignalled())
	{
		object.Observed();
		ArchRestoreFlags(flags);
		return KWaitResult::Signaled;
	}

	const nano100_t tscStart = m_readTsc();
	const nano100_t deadline = tscStart + ToNano(timeout) / 100;

	//Sched.Signal and Sched.Timeout must be written before State=SignalWait so any Schedule() seeing SignalWait finds a valid Signal
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
	//Kernel threads (e.g. boot, when init.exe fails to start) have no UserThread and are never in the target
	const UThread* const user = GetCurrentThread().UserThread;
	const bool isRunning = user && &user->Process == &process;

	for (UThread& thread : process.Threads)
	{
		//Skip threads that already exited (their KThread is gone)
		if (thread.Thread)
			thread.Thread->Sched.State = KThreadState::Terminated;
	}

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
