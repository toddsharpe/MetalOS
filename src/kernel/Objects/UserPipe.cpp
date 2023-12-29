#include "UserPipe.h"

#include "Kernel/MetalOS.Kernel.h"
#include "Kernel/Kernel.h"
#include <vector>

bool UserPipe::EventSignal(void* const arg)
{
	PipeEvent* event = ((PipeEvent*)arg);
	return event->Pipe.EventSignal(event->Op, event->Count);
}

UserPipe::UserPipe() :
	Readers(),
	Writers(),
	m_readIndex(),
	m_writeIndex(),
	m_count(),
	m_size(PageSize * PageCount),
	m_buffer()
{
	Init();
}

UserPipe::~UserPipe()
{
	//todo: free/unmap pages
}

void UserPipe::Init()
{
	std::vector<paddr_t> addresses;
	const paddr_t address = kernel.AllocatePhysical(PageCount);

	//Double map
	for (size_t i = 0; i < PageCount; i++)
	{
		addresses.push_back(address + ((i + 1) << PageShift));
	}
	for (size_t i = 0; i < PageCount; i++)
	{
		addresses.push_back(address + ((i + 1) << PageShift));
	}

	m_buffer = kernel.VirtualMap(nullptr, addresses);
	memset(m_buffer, 0, m_count);
}

//If pipe isnt broken, returns true only if full read is satisfied.
//If pipe is broken, does a partial and returns true (even if no bytes were read)
bool UserPipe::Read(void* const buffer, const size_t length, size_t& read)
{
	if (!IsBroken() && m_count < length)
		return false;

	//Determine size to read. Will be length if pipe isn't broken (checked above)
	read = std::max(m_count, length);

	//Read data
	const void* source = MakePointer<void*>(m_buffer, m_readIndex);
	memcpy(buffer, source, length);
	m_readIndex += length;
	m_count -= length;
	return true;
}

bool UserPipe::Write(const void* buffer, const size_t length)
{
	if (m_count + length > m_size)
		return false;

	void* destination = MakePointer<void*>(m_buffer, m_writeIndex);
	memcpy(destination, buffer, length);
	m_writeIndex += length; 
	m_count += length;
	return true;
}

size_t UserPipe::FreeSpace() const
{
	return m_size - m_count;
}

size_t UserPipe::Count() const
{
	return m_count;
}

bool UserPipe::IsBroken() const
{
	return (Readers == 0) || (Writers == 0);
}

bool UserPipe::EventSignal(const PipeOp op, const size_t count)
{
	if (IsBroken())
		return true;

	switch (op)
	{
		case PipeOp::Read:
			return Count() >= count;

		case PipeOp::Write:
			return FreeSpace() >= count;

		default:
			Fatal("Invalid pipe op");
	}

	return false;
}
