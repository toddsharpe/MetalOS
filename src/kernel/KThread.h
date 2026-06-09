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
class Scheduler;
class KThread
{
	friend class Scheduler;

public:
	KThread(const KThreadStart start, void* const arg, const CString& name);

	void Init(void* const entry);
	void Start();

	void Display() const;

	void* ContextPtr;
	UThread* UserThread;
	const uint32_t Id;
	KProcess* Process;

private:
	static uint32_t LastId;
	static constexpr size_t StackSize = Arch::PageSize << 8; // 1MB stack

	const KThreadStart m_start;
	void* const m_arg;
	void* m_stack;
	void* m_stackPointer;

	//Scheduler
	KThreadState m_state;
	KWaitResult m_waitResult;
	nano100_t m_timeout;
	KSignal* m_signal;
	nano_t m_scheduleTime;
	nano_t m_totalCpuTime;

	//Storage
	Arch::Context m_context;
	StaticString<32> m_name; //TODO(tsharpe): Shouldnt be a static string, just string
};

//Assert assembly offsets
static_assert(offsetof(KThread, ContextPtr) == 0, "x64_SYSTEMCALL asm invalid");
static_assert(offsetof(KThread, UserThread) == 8, "x64_SYSTEMCALL asm invalid");
