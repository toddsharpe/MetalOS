#pragma once

#include "UserProcess.h"
#include <shared/MetalOS.System.h>

//TODO(tsharpe): Deallocate TEB from proces
//TODO(tsharpe): Unmap stack
class UserThread
{
public:
	static bool HasMessage(void* const arg) { return ((UserThread*)arg)->HasMessage(); };

	UserThread(const ThreadStart startAddress, void* const arg, void* const entry, const size_t stackSize, UserProcess& process);

	void Run();

	void EnqueueMessage(const Message& message);
	bool DequeueMessage(Message& message);
	bool HasMessage() const;

	void Display();
	void DisplayMessages();
	void DisplayDetails();

	void* Stack;
	bool Deleted;
	const uint32_t Id;
	UserProcess& Process;

private:
	static constexpr size_t DefaultStack = (PageSize << 4); //64K
	static uint32_t LastId;

	ThreadEnvironmentBlock* m_teb;
	X64_CONTEXT m_context;
	void* m_stackAllocation;
	std::list<Message> m_messages;

	::NO_COPY_OR_ASSIGN(UserThread);
};
