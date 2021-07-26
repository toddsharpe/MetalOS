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

