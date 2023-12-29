#pragma once

#include "Kernel/MetalOS.Kernel.h"
#include "UserThread.h"
#include "Kernel/Objects/KSignalObject.h"
#include "user/MetalOS.Types.h"
#include <cstdint>

enum class ThreadState
{
	Ready,
	Running,
	SignalWait,
	Sleeping,
	Terminated
};

class KThread
{
	friend class Scheduler;

public:
	KThread(const ThreadStart start, void* const arg);
	~KThread();

	void Init(void* const entry);
	void Run();
	
	void Display() const;

	const uint32_t Id;

	X64_CONTEXT* Context;
	UserThread* UserThread;
	std::string Name;

private:
	static uint32_t LastId;
	static constexpr size_t StackPages = 8;
	
	const ThreadStart m_start;
	void* const m_arg;
	void* m_stack;
	void* m_stackPointer;

	//Scheduler
	ThreadState m_state;
	WaitStatus m_waitStatus;
	nano100_t m_timeout;
	KSignalObject* m_signal;

	::NO_COPY_OR_ASSIGN(KThread);
};
