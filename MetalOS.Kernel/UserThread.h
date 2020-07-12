#pragma once

#include "UserProcess.h"

class UserThread
{
public:
	static uint32_t LastId;

	UserThread(ThreadStart startAddress, void* arg, void* stack, void* entry, UserProcess& process);

	UserProcess& GetProcess()
	{
		return m_process;
	}

	void* GetContext()
	{
		return m_context;
	}

	ThreadEnvironmentBlock* GetTEB() const
	{
		return m_teb;
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
	void* m_context;
	std::list<Message*> m_messages;
};

