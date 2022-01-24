#include "Kernel.h"
#include "Assert.h"

#include "MsfStream.h"

MsfStream::MsfStream(const std::vector<uint32_t>& blockNumbers, const uint32_t size, const MsfFile& file) :
	m_blockNumbers(blockNumbers),
	m_size(size),
	m_file(file),
	m_position()
{

}

void MsfStream::Display() const
{
	if (m_blockNumbers.size() == 0)
		kernel.Printf("Empty stream");
	else
		kernel.Printf("Start: 0x%x End:0x%x Size: 0x%x\n", m_blockNumbers[0], m_blockNumbers[m_blockNumbers.size() - 1], m_size);
}

bool MsfStream::ReadString(std::string& value)
{
	char* blockPointer = (char*)GetPointer();
	while ((*blockPointer) != '\0')
	{
		value.append(1, *blockPointer);

		m_position++;
		blockPointer = (char*)GetPointer();
	}

	m_position++;
	return true;
}

void MsfStream::Read(char* buffer, size_t length)
{
	char* blockPointer = (char*)GetPointer();
	for (size_t i = 0; i < length; i++)
	{
		*buffer = *blockPointer;

		buffer++;
		m_position++;
		blockPointer = (char*)GetPointer();
	}
}

bool MsfStream::ReadString(char* str)
{
	char* blockPointer = (char*)GetPointer();
	while ((*blockPointer) != '\0')
	{
		*str = *blockPointer;

		m_position++;
		str++;
		blockPointer = (char*)GetPointer();
	}
	*str = '\0';

	m_position++;
	return true;
}

void MsfStream::SkipString()
{
	char* blockPointer = (char*)GetPointer();
	while ((*blockPointer) != '\0')
	{
		m_position++;
		blockPointer = (char*)GetPointer();
	}

	m_position++;
}

void* MsfStream::GetPointer() const
{
	const size_t blockNumber = m_position / m_file.BlockSize();
	const size_t blockIndex = m_position % m_file.BlockSize();

	const void* block = m_file.GetBlock(m_blockNumbers[blockNumber]);
	return (void*)((uintptr_t)block + blockIndex);
}
