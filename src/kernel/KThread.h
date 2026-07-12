#pragma once

#include <cstdint>
#include <cstddef>
#include "kernel/UThread.h"
#include "kernel/Objects/KSignal.h"

enum class KThreadState
{
	Ready,
	Running,
	SignalWait,
	Sleeping,
	Terminated
};

class KProcess;
class KThread
{
public:
	//Scheduler-owned state. Public so the scheduler needs no friendship; the rest of KThread
	//stays private. The scheduler is the only writer.
	struct SchedState
	{
		KThreadState State = KThreadState::Ready;
		KWaitResult WaitResult = KWaitResult::None;
		nano100_t Timeout = 0;
		KSignal* Signal = nullptr;
		nano_t ScheduleTime = 0;
		nano_t TotalCpuTime = 0;
	};

	KThread(const KThreadStart start, void* const arg, const CString& name);

	void Init(void* const entry);
	void Start();

	void Display() const;

	//Top of this thread's kernel stack, loaded as the interrupt stack on switch-in.
	void* InterruptStack() const { return m_stackPointer; }

	void* ContextPtr;
	UThread* UserThread;
	const uint32_t Id;
	KProcess* Process;
	SchedState Sched;

private:
	static uint32_t LastId;
	static constexpr size_t StackSize = Arch::PageSize << 8; // 1MB stack

	const KThreadStart m_start;
	void* const m_arg;
	void* m_stack;
	void* m_stackPointer;

	//Storage
	Arch::Context m_context;
	StaticString<32> m_name; //TODO(tsharpe): Shouldnt be a static string, just string
};

//Assert assembly offsets
static_assert(offsetof(KThread, ContextPtr) == 0, "x64_SYSTEMCALL asm invalid");
static_assert(offsetof(KThread, UserThread) == 8, "x64_SYSTEMCALL asm invalid");
