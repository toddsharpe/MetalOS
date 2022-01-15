#include "Kernel.h"
#include "Assert.h"
#include <coreclr/cvinfo.h>

#include "PdbDbi.h"
#include "PortableExecutable.h"

PdbDbi::PdbDbi(MsfStream& stream, MsfFile& file, void* loadedAddress) :
	m_stream(stream),
	m_file(file),
	m_publics(),
	m_header(),
	m_textSection()
{
	Assert(m_stream.Read(&m_header));
	Assert(m_header.VersionSignature == -1);
	Assert(m_header.VersionHeader == DbiStreamVersion::V70);
	Assert(m_header.Machine == 0x8664);

	m_textSection = PortableExecutable::GetPESection(TextSection - 1, (uintptr_t)loadedAddress);
	Assert(strcmp((char*)m_textSection->Name, ".text") == 0);

	m_publics.Load(m_file.GetStream(m_header.SymRecordStreamIndex));
}

#define MAXPATH 255
bool PdbDbi::ResolveFunction(const uint32_t rva, PdbFunctionLookup& lookup)
{
	const uint32_t offset = rva - m_textSection->VirtualAddress;
	//kernel.Printf("Text VA: 0x%016x, Off: 0x%016x\n", m_textSection->VirtualAddress, offset);

	uint16_t moduleIndex;
	Assert(GetModuleIndex(TextSection, offset, moduleIndex));
	//kernel.Printf("ModuleIndex: %d\n", moduleIndex);

	ModInfo info;
	Assert(GetModuleInfo(moduleIndex, info));
	Assert(info.C11ByteSize == 0);
	Assert(info.C13ByteSize != 0);

	char buffer[MAXPATH];
	Assert(m_stream.ReadString(buffer)); //Module
	Assert(m_stream.ReadString(buffer)); //Object

	MsfStream& moduleStream = m_file.GetStream(info.ModuleSymStream);
	uint32_t header = moduleStream.Read<uint32_t>();
	Assert(header == 4);//TODO: magic
	moduleStream.Seek(info.SymByteSize);

	{
		CV_DebugSSubsectionHeader_t checksumHeader;
		Assert(moduleStream.Read(&checksumHeader));
		//if (checksumHeader.type != DEBUG_S_SUBSECTION_TYPE::DEBUG_S_FILECHKSMS)
		//{
		//	kernel.Printf("Unexpected checksum type: 0x%x\n", checksumHeader.type);
		//	Assert(false);
		//}

		//Skip checksums section
		moduleStream.Skip(checksumHeader.cbLen);

		const uint32_t endPosition = info.SymByteSize + info.C13ByteSize;
		bool found = false;
		while (moduleStream.Position() < endPosition)
		{
			CV_DebugSSubsectionHeader_t subHeader;
			Assert(moduleStream.Read(&subHeader));
			//AssertEqual(subHeader.type, DEBUG_S_SUBSECTION_TYPE::DEBUG_S_LINES);
			if (subHeader.type != DEBUG_S_SUBSECTION_TYPE::DEBUG_S_LINES)
			{
				lookup.Name = "<unknown type>";
				lookup.LineNumber = subHeader.type;
				return false;
			}

			CV_DebugSLinesHeader_t linesHeader;
			Assert(moduleStream.Read(&linesHeader));

			//kernel.Printf("%04x:%08x-%08x\n", 1, linesHeader.offCon, (linesHeader.offCon + linesHeader.cbCon));
			if (offset < linesHeader.offCon || offset >= (linesHeader.offCon + linesHeader.cbCon))
			{
				moduleStream.Skip(subHeader.cbLen - sizeof(CV_DebugSLinesHeader_t));
			}
			else
			{
				CV_DebugSLinesFileBlockHeader_t fileHeader;
				Assert(moduleStream.Read(&fileHeader));

				const uint32_t functionOffset = offset - linesHeader.offCon;

				CV_Line_t prev = { };
				for (size_t i = 0; fileHeader.nLines; i++)
				{
					CV_Line_t line;
					Assert(moduleStream.Read(&line));

					if (functionOffset >= prev.offset && functionOffset < line.offset)
					{
						lookup.Name = m_publics.GetFunction(linesHeader.offCon);
						lookup.LineNumber = line.linenumStart;

						found = true;
						break;
					}

					prev = line;
				}

				if (found)
					break;
			}
		}
	}

	return false;
}

bool PdbDbi::GetModuleIndex(const uint32_t section, const uint32_t offset, uint16_t& moduleIndex)
{
	//Seek to start of section contribution stream
	m_stream.Seek(sizeof(DbiStreamHeader) + m_header.ModInfoSize);
	const uint32_t endPosition = m_stream.Position() + m_header.ModInfoSize + m_header.SectionContributionSize;

	//Reassert this here
	SectionContrSubstreamVersion version;
	Assert(m_stream.Read(&version));
	AssertEqual(version, SectionContrSubstreamVersion::Ver60); //V2 uses SectionContribEntry2
	SectionContribEntry entry;

	while (m_stream.Position() < endPosition)
	{
		Assert(m_stream.Read<SectionContribEntry>(&entry));

		if (entry.Section != section)
			continue;

		if (entry.Offset <= offset && (entry.Offset + entry.Size) > offset)
		{
			//kernel.PrintBytes((char*)&entry, sizeof(SectionContribEntry));
			moduleIndex = entry.ModuleIndex;
			return true;
		}
	}
	
	//kernel.Printf("Section: %d Offset: 0x%08x. start: 0x%08x, end: 0x%08x\n", section, offset, m_stream.Position(), endPosition);
	Assert(false);
	return false;
}

bool PdbDbi::GetModuleInfo(const uint32_t index, ModInfo& info)
{
	m_stream.Seek(sizeof(DbiStreamHeader));
	const uint32_t endPosition = m_stream.Position() + m_header.ModInfoSize;

	uint32_t current = 0;
	while (m_stream.Position() < endPosition)
	{
		Assert(m_stream.Read(&info));

		if (current == index)
			return true;

		current++;

		m_stream.SkipString();
		m_stream.SkipString();
		m_stream.SkipAlign<uint32_t>();
	}
	
	return false;
}
