#pragma once

#include "UserProcess.h"

class Scheduler;
class UserThread
{
	friend class Scheduler;

public:
	static uint32_t LastId;
	const static size_t DefaultStack = (PAGE_SIZE << 4); //64K

	UserThread(ThreadStart startAddress, void* arg, void* entry, size_t stackSize, UserProcess& process);

	void Run();

	UserProcess& GetProcess()
	{
		return m_process;
	}

	bool HasMessage()
	{
		return !m_messages.empty();
	}

	const uint32_t GetId() const
	{
		return m_id;
	}

	Message* DequeueMessage();
	void EnqueueMessage(Message* message);


	void Display();
	void DisplayMessages();
	void DisplayDetails();

	void* SavedStack;
	//void* SavedUserRBP;
	bool Deleted;

private:
	uint32_t m_id;
	UserProcess& m_process;
	ThreadEnvironmentBlock* m_teb;
	void* m_stackAllocation;//Points to top of stack
	void* m_context;

	std::list<Message*> m_messages;
};

