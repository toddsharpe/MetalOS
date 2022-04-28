#pragma once
#include <cstdint>
#include "MetalOS.Kernel.h"
#include "UserThread.h"
#include "KSignalObject.h"

class KThread : public KSignalObject
{
	friend class Scheduler;

private:
	static size_t LastId;
	static const size_t StackPages = 8;

public:
	KThread(const ThreadStart start, void* arg, UserThread* userThread = nullptr);
	~KThread();

	void Run();
	void InitContext(void* entry);

	uint32_t GetId() const;
	void* GetStackPointer() const;
	UserThread* GetUserThread() const;

	void Display() const;

	virtual bool IsSignalled() const override;

	virtual void Dispose() override;

private:
	//Must be kept the first members of this struct, or insync with SystemCall.asm
	void* m_context;
	UserThread* m_userThread;

	ThreadStart m_start;
	void* m_arg;
	const size_t m_id;
	void* m_stackPointer; //Points to bottom of stack minus ArchStackReserve
	void* m_stackAllocation;//Points to top of stack

	//Scheduler
	ThreadState m_state;
	WaitStatus m_waitStatus;
	nano100_t m_sleepWake;

public:
	std::string Name;
};

