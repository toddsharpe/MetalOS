#include "UserPipe.h"

#include "MetalOS.Kernel.h"
#include "Kernel.h"
#include <vector>

UserPipe::UserPipe() :
	KObject(KObjectType::Pipe, false),
	m_readIndex(),
	m_writeIndex(),
	m_count(),
	m_size(PAGE_SIZE* PageCount),
	m_buffer(),
	ReaderCount(),
	WriterCount()
{
	std::vector<paddr_t> addresses;
	paddr_t page = kernel.AllocatePhysical(PageCount);
	addresses.push_back(page);
	addresses.push_back(page);

	m_buffer = kernel.VirtualMap(nullptr, addresses, MemoryProtection::PageReadWrite);
	memset(m_buffer, 0, PAGE_SIZE * PageCount);
}

bool UserPipe::Read(void* buffer, size_t length)
{
	if (m_count < length)
		return false;

	const void* source = MakePointer<void*>(m_buffer, m_readIndex);
	memcpy(buffer, source, length);
	m_readIndex += length;
	m_count -= length;
	return true;
}

size_t UserPipe::ReadPartial(void* buffer, size_t length)
{
	size_t read = std::max(m_count, length);
	const void* source = MakePointer<void*>(m_buffer, m_readIndex);
	memcpy(buffer, source, read);
	m_readIndex += read;
	m_count -= read;
	return read;
}

bool UserPipe::Write(const void* buffer, size_t length)
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

void UserPipe::Dispose()
{
	//todo: free/unmap pages
}
