#pragma once

#include "core_crt/stdint.h"
#include "Lib/StaticVector.h"
#include "Lib/List.h"
#include "kernel/Arch.h"
#include "kernel/Pdb/MsfStream.h"
#include "Assert.h"

// https://llvm.org/docs/PDB/MsfFile.html
// https://github.com/microsoft/microsoft-pdb/blob/master/PDB/msf/msf.cpp
//Only supports 64-bit PDBs


class MsfFile
{
public:

	static void* Read(void* const context, const uint32_t blockNumber)
	{
		return reinterpret_cast<MsfFile*>(context)->GetBlock(blockNumber);
	}

	MsfFile(const void* const image) :
		m_image(image),
		m_header(),
		m_streams()
	{
		
	}

	void Initialize(Arena& arena)
	{
		ListInitializeHead(m_streams);
		
		//Set header
		m_header = static_cast<const Header*>(m_image);
		
		AssertEqual(memcmp((void*)m_image, &Magic, sizeof(Magic)), 0);
		AssertEqual(m_header->BlockSize, Arch::PageSize);

		//Create directory
		void* allocated = arena.Allocate(sizeof(MsfStream));
		Assert(allocated);
		MsfStream* directory = new (allocated) MsfStream(m_header->BlockSize, Read, this);
		directory->Size = m_header->NumDirectoryBytes;

		//Load directory mapping, a list of block numbers for the directory stream
		const size_t numDirBlocks = x64::SizeToPages(m_header->NumDirectoryBytes);
		const uint32_t* dirBlockNum = (uint32_t*)GetBlock(m_header->BlockMapAddr);
		for (size_t i = 0; i < numDirBlocks; i++)
		{
			directory->BlockNumbers.Add(*dirBlockNum);
			dirBlockNum++;
		}

		//Read directory stream contents
		const uint32_t numStreams = directory->Read<uint32_t>();
		AssertNotEqual(numStreams, 0);

		//Load stream sizes
		//NOTE(tsharpe): PDF observed with stream size of 0xFFFFFFFF. Not sure what it means, skip for now.
		for (size_t i = 0; i < numStreams; i++)
		{
			void* mem = arena.Allocate(sizeof(MsfStream));
			Assert(mem);
			MsfStream* stream = new (mem) MsfStream(m_header->BlockSize, Read, this);
			ListInsertTail(m_streams, stream->Link);
			
			const uint32_t streamSize = directory->Read<uint32_t>();
			if (streamSize == 0xFFFFFFFF)
				stream->IsInvalid = true;
			else
			{
				stream->Size = streamSize;
				stream->BlockCount = x64::SizeToPages(streamSize);
			}
		}

		//Load stream blocks
		ListForEach<MsfStream, MsfStream*>(m_streams, [](const ListEntry& entry, MsfStream& item, MsfStream* ctx)
		{
			if (item.IsInvalid)
				return;

			for (size_t i = 0; i < item.BlockCount; i++)
			{
				const uint32_t blockNumber = ctx->Read<uint32_t>();
				AssertNotEqual(blockNumber, 0);
				item.BlockNumbers.Add(blockNumber);
			}
		}, directory);
	}

	MsfStream& GetStream(const uint32_t index)
	{
		Assert(index < m_streams.Count);

		MsfStream* stream = ListGet<MsfStream>(m_streams, index);
		Assert(stream);
		stream->Seek(0);

		return *stream;
	}

	void* GetBlock(const uint32_t blockNumber) const
	{
		return (void*)(((uintptr_t)m_header) + ((uintptr_t)blockNumber << Arch::PageShift));
	}

	uint32_t BlockSize() const
	{
		return m_header->BlockSize;
	}

	void Display() const
	{
		Printf("MsfFile::Display\n");
		Printf("  Header:\n");
		Printf("    BlockSize: 0x%x\n", m_header->BlockSize);
		Printf("    FreeBlockMapBlock: 0x%x\n", m_header->FreeBlockMapBlock);
		Printf("    NumBlocks: 0x%x\n", m_header->NumBlocks);
		Printf("    NumDirectoryBytes: 0x%x\n", m_header->NumDirectoryBytes);
		Printf("    Unknown: 0x%x\n", m_header->Unknown);
		Printf("    BlockMapAddr: 0x%x\n", m_header->BlockMapAddr);
		Printf("  Streams: 0x%x\n", m_streams.Count);
		ListForEach<MsfStream>(m_streams, [](const ListEntry&, const MsfStream& stream)
		{
			stream.Display();
		});
	}

private:
	static constexpr char Magic[0x1e] = "Microsoft C/C++ MSF 7.00\r\n\x1a\x44\x53";

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

	const void* const m_image;
	const Header* m_header;
	ListHead m_streams;
};

