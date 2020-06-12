#include "MsfFile.h"
#include "Main.h"
#include "crt_string.h"
#include <array>
#include "MsfFile.h"

const char MsfFile::Magic[0x1e] = "Microsoft C/C++ MSF 7.00\r\n\x1a\x44\x53";

MsfFile::MsfFile(uintptr_t base) :
	m_header((Header*)base),
	m_streams()
{
	Assert(memcmp((void*)base, &Magic, sizeof(Magic)) == 0);
	Assert(m_header->BlockSize == PAGE_SIZE);

	//Load directory mapping, a list of block numbers for the directory stream
	std::vector<uint32_t> blockNumbers;
	{
		uint32_t* directory = (uint32_t*)GetBlock(m_header->BlockMapAddr);
		while (*directory != 0)
		{
			blockNumbers.push_back(*directory);
			directory++;
		}
	}

	//Load directory stream
	MsfStream directory(blockNumbers, m_header->NumDirectoryBytes, *this);
	const uint32_t numStreams = directory.Read<uint32_t>();
	Assert(numStreams != 0);

	std::vector<Stream> streams;
	streams.resize(numStreams);

	//Load stream sizes
	for (size_t i = 0; i < numStreams; i++)
	{
		const uint32_t streamSize = directory.Read<uint32_t>();
		streams[i].Size = streamSize;
		streams[i].BlockNumbers.resize(SIZE_TO_PAGES(streamSize));
	}

	//Load stream blocks
	for (auto& stream : streams)
	{
		for (size_t i = 0; i < stream.BlockNumbers.size(); i++)
		{
			const uint32_t blockNumber = directory.Read<uint32_t>();
			Assert(blockNumber != 0);
			stream.BlockNumbers[i] = blockNumber;
		}
	}

	//Create MsfStreams
	m_streams.reserve(numStreams);
	for (const auto& stream : streams)
	{
		MsfStream stream(stream.BlockNumbers, stream.Size, *this);
		m_streams.push_back(stream);
	}
}

MsfStream& MsfFile::GetStream(const uint32_t index)
{
	Assert(index < m_streams.size());
	m_streams[index].Seek(0);
	return m_streams[index];
}

void* MsfFile::GetBlock(const uint32_t blockNumber) const
{
	return (void*)(((uintptr_t)m_header) + ((uintptr_t)blockNumber << PAGE_SHIFT));
}

void MsfFile::Display()
{
	Print("MsfFile::Display\n");
	Print("  Header:\n");
	Print("    BlockSize: 0x%x\n", m_header->BlockSize);
	Print("    FreeBlockMapBlock: 0x%x\n", m_header->FreeBlockMapBlock);
	Print("    NumBlocks: 0x%x\n", m_header->NumBlocks);
	Print("    NumDirectoryBytes: 0x%x\n", m_header->NumDirectoryBytes);
	Print("    Unknown: 0x%x\n", m_header->Unknown);
	Print("    BlockMapAddr: 0x%x\n", m_header->BlockMapAddr);
	Print("  Streams: 0x%x\n", m_streams.size());
	for (size_t i = 0; i < m_streams.size(); i++)
	{
		Print("    [0x%x]: ", i);
		
		//const Stream& stream = m_streams[i];
		//for (size_t j = 0; j < stream.BlockNumbers.size(); j++)
		//{
		//	Print("%x,", stream.BlockNumbers[j]);
		//	if (j != 0 && j % 16 == 0)
		//		Print("\n");
		//}
		//Print("\n");
	}
}
