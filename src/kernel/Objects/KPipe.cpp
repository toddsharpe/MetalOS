#include "kernel/Objects/KPipe.h"
#include "kernel/Arch.h"
#include "kernel/Api.h"
#include "Lib/StaticVector.h"

bool KPipe::EventSignal(void* const arg)
{
	KPipeSignal* event = reinterpret_cast<KPipeSignal*>(arg);
	return event->Pipe.EventSignal(event->Op, event->Count);
}

constexpr KPipe::KPipe(const size_t size) :
	Readers(),
	Writers(),
	m_buffer(),
	m_count(),
	m_size(size),
	m_readIndex(),
	m_writeIndex()
{
	
}

void KPipe::Init()
{
	Assert(m_size % PageSize == 0);
	const size_t count = SizeToPages(m_size);
	
	//Acquire physical pages
	StaticVector<paddr_t, 32> addresses;
	for (size_t i = 0; i < count; i++)
	{
		const paddr_t addr = KePhysicalAlloc();
		addresses.Add(addr);
	}

	//Add them to list twice (double map)
	for (size_t i = 0; i < count; i++)
	{
		const paddr_t addr = addresses[i];
		addresses.Add(addr);
	}

	//Map into kernel
	void* const base = KeVirtualMap(addresses.begin(), addresses.Count());
	m_buffer = reinterpret_cast<uint8_t*>(base);
}

bool KPipe::Read(void* const buffer, const size_t length, size_t& bytesRead)
{
	bytesRead = 0;
	if (IsEmpty())
		return true;

	const size_t logicalWrite = m_writeIndex > m_readIndex ? m_writeIndex : m_writeIndex + m_size;
	const size_t available = logicalWrite - m_readIndex;

	bytesRead = Clamp<size_t>(available, 0, length);
	memcpy(buffer, &m_buffer[m_readIndex], bytesRead);
	m_readIndex = (m_readIndex + bytesRead) % m_size;
	m_count -= bytesRead;

	return true;
}

bool KPipe::Write(const void* buffer, const size_t length, size_t& bytesWritten)
{
	bytesWritten = 0;
	if (IsFull())
		return false;

	const size_t logicalRead = m_readIndex > m_writeIndex ? m_readIndex : m_readIndex + m_size;
	const size_t available = logicalRead - m_writeIndex - 1;

	if (available < length)
		return false;

	bytesWritten = length;
	memcpy(&m_buffer[m_writeIndex], buffer, bytesWritten);
	m_writeIndex = (m_writeIndex + bytesWritten) % m_size;
	m_count += bytesWritten;

	return true;
}

//TODO(tsharpe): This method should be renamed printf, and kernel routine named KePrintf
void KPipe::Print(const char* format, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, format);
	int retval = vsprintf(buffer, format, args);
	va_end(args);
	buffer[retval] = '\0';

	size_t bytesWritten = 0;
	Write(buffer, retval, bytesWritten);
}

size_t KPipe::Count() const
{
	return m_count;
}

size_t KPipe::Space() const
{
	return m_size - m_count - 1;
}

bool KPipe::IsBroken() const
{
	return Readers.IsEmpty() || Writers.IsEmpty();
}

void KPipe::Display() const
{
	Printf("KPipe::Display\n");
	Printf("  Buffer: 0x%016x\n", m_buffer);
	Printf("   Count: 0x%016x\n", m_count);
	Printf("    Size: 0x%016x\n", m_size);
	Printf("    Read: 0x%016x\n", m_readIndex);
	Printf("   Write: 0x%016x\n", m_writeIndex);
}

bool KPipe::EventSignal(const KPipeOp op, const size_t count)
{
	if (IsBroken())
		return true;

	switch (op)
	{
		case KPipeOp::Read:
			return Count() >= count;

		case KPipeOp::Write:
			return Space() >= count;

		default:
			Fatal("Invalid pipe op");
			return false;
	}
}

bool KPipe::IsEmpty() const
{
	return m_readIndex == m_writeIndex;
}

bool KPipe::IsFull() const
{
	return (m_writeIndex + 1) % m_size == m_readIndex;
}
