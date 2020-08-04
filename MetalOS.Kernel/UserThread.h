#pragma once

#include "UserProcess.h"

class Scheduler;
class UserThread
{
public:
	static uint32_t LastId;

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

	Message* DequeueMessage();
	void EnqueueMessage(Message* message);

	//Thread can have one window for now
	UserWindow* Window;

	void Display();
	void DisplayMessages();
	void DisplayDetails();

private:
	uint32_t m_id;
	UserProcess& m_process;
	ThreadEnvironmentBlock* m_teb;
	void* m_stackAllocation;//Points to top of stack
	void* m_context;

	std::list<Message*> m_messages;
};

