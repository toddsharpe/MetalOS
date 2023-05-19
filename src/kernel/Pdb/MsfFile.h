#pragma once


#include <cstdint>
#include <vector>

// https://llvm.org/docs/PDB/MsfFile.html
// https://github.com/microsoft/microsoft-pdb/blob/master/PDB/msf/msf.cpp
//Only supports 64-bit PDBs

class MsfStream;
class MsfFile
{
public:
	MsfFile(const void* base);

	MsfStream& GetStream(const uint32_t index);
	void* GetBlock(const uint32_t blockNumber) const;

	uint32_t BlockSize() const
	{
		return m_header->BlockSize;
	}

	void Display() const;

private:
	static const char Magic[0x1e];

	struct Header
	{
		char FileMagic[sizeof(Magic)];
		uint32_t BlockSize;
		uint32_t FreeBlockMapBlock;
		uint32_t NumBlocks;
		uint32_t NumDirectoryBytes; //Size in bytes of stream directory
		uint32_t Unknown;
		uint32_t BlockMapAddr; //Index of block containing block numbers for stream directory
	};

	/* Reference structure
	struct StreamDirectory
	{
		uint32_t NumStreams;
		uint32_t StreamSizes[NumStreams];
		uint32_t StreamBlocks[NumStreams][];
	};
	*/

	struct Stream
	{
		Stream() :
			Size(),
			Invalid(),
			BlockNumbers()
		{}

		uint32_t Size;
		bool Invalid;
		std::vector<uint32_t> BlockNumbers;
	};


	const Header* m_header;
	std::vector<MsfStream> m_streams;
};

