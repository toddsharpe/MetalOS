#include "Assert.h"
#include <coreclr/cvinfo.h>

#include "PdbDbi.h"
#include "Kernel/PortableExecutable.h"
#include <Kernel/Kernel.h>

PdbDbi::PdbDbi(MsfStream& stream, MsfFile& file, const Handle image) :
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

	m_textSection = PortableExecutable::GetPESection(image, TextSection - 1);
	Assert(strcmp((char*)m_textSection->Name, ".text") == 0);

	m_publics.Load(m_file.GetStream(m_header.SymRecordStreamIndex));
}

#define MAXPATH 255
bool PdbDbi::ResolveFunction(const uint32_t rva, PdbFunctionLookup& lookup)
{
	const uint32_t offset = rva - m_textSection->VirtualAddress;
	//Printf("Text VA: 0x%016x, Off: 0x%016x\n", m_textSection->VirtualAddress, offset);

	uint16_t moduleIndex;
	Assert(GetModuleIndex(TextSection, offset, moduleIndex));
	//Printf("ModuleIndex: %d\n", moduleIndex);

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

	//After seeking past the symbols, PDB contains records in order:
	// * CV_DebugSLinesHeader_t (DEBUG_S_LINES) (or DEBUG_S_FILECHKSMS, not handled here)
	// * CV_DebugSLinesHeader_t - defines range of offsets in segment
	// * CV_DebugSLinesFileBlockHeader_t - defines number of lines
	// * CV_Line_t - defines each line entry
	{
		//Entries are sorted
		const uint32_t endPosition = info.SymByteSize + info.C13ByteSize;
		while (moduleStream.Position() < endPosition)
		{
			CV_DebugSSubsectionHeader_t sectionHeader;
			Assert(moduleStream.Read(&sectionHeader));

			if (sectionHeader.type != DEBUG_S_SUBSECTION_TYPE::DEBUG_S_LINES)
			{
				moduleStream.Skip(sectionHeader.cbLen);
				continue;
			}

			CV_DebugSLinesHeader_t linesHeader;
			Assert(moduleStream.Read(&linesHeader));

			//Printf("%04x:%08x-%08x\n", 1, linesHeader.offCon, (linesHeader.offCon + linesHeader.cbCon));

			//Offset wasn't found (list is sorted)
			if (offset < linesHeader.offCon)
			{
				//not found
				lookup.Name = "<not found1>";
				lookup.LineNumber = 0;
				return false;
			}

			//Continue to next entry
			if (offset >= (linesHeader.offCon + linesHeader.cbCon))
			{
				moduleStream.Skip(sectionHeader.cbLen - sizeof(CV_DebugSLinesHeader_t));
				continue;
			}

			//Iterate through blocks
			CV_DebugSLinesFileBlockHeader_t fileHeader;
			Assert(moduleStream.Read(&fileHeader));

			//Relative offset in lines section
			const uint32_t targetOffset = offset - linesHeader.offCon;
			//Printf("Offset: 0x%016x, offCon: 0x%016x\n", offset, linesHeader.offCon);
			CV_Line_t prev = {};
			for (size_t i = 0; fileHeader.nLines; i++)
			{
				CV_Line_t line;
				Assert(moduleStream.Read(&line));

				//Lines are comprised of one or more instructions
				//Check if target offset is between start of line and start of next line
				if (targetOffset >= prev.offset && targetOffset < line.offset)
				{
					m_publics.GetFunction(linesHeader.offCon, lookup.Name);
					lookup.LineNumber = prev.linenumStart;
					return true;
				}
				prev = line;
			}
		}
	}

	lookup.Name = "<not found2>";
	lookup.LineNumber = 0;
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
			//PrintBytes((char*)&entry, sizeof(SectionContribEntry));
			moduleIndex = entry.ModuleIndex;
			return true;
		}
	}
	
	Printf("Section: %d Offset: 0x%08x. start: 0x%08x, end: 0x%08x\n", section, offset, m_stream.Position(), endPosition);
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
