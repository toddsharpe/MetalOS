#include "PdbDbi.h"
#include "Main.h"

#include "RuntimeSupport.h"

PdbDbi::PdbDbi(MsfStream& stream, MsfFile& file) :
	m_stream(stream),
	m_file(file),
	m_publics(),
	m_header(),
	m_kernelText()
{
	Assert(m_stream.Read(&m_header));
	Assert(m_header.VersionSignature == -1);
	Assert(m_header.VersionHeader == DbiStreamVersion::V70);
	Assert(m_header.Machine == 0x8664);

	m_kernelText = RuntimeSupport::GetPESection(TextSection - 1, KernelBaseAddress);
	Assert(strcmp((char*)m_kernelText->Name, ".text") == 0);

	m_publics.Load(m_file.GetStream(m_header.SymRecordStreamIndex));
}

#define MAXPATH 255
bool PdbDbi::PrintStack(const uint32_t rva)
{
	const uint32_t offset = rva - m_kernelText->VirtualAddress;

	SectionContribEntry2 contribution;
	Assert(GetSectionContribution(TextSection, offset, contribution));

	ModInfo info;
	Assert(GetModuleInfo(contribution.SC.ModuleIndex, info));
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
		Assert(checksumHeader.type == DEBUG_S_SUBSECTION_TYPE::DEBUG_S_FILECHKSMS);

		//Skip checksums section
		moduleStream.Skip(checksumHeader.cbLen);

		const uint32_t endPosition = info.SymByteSize + info.C13ByteSize;
		bool found = false;
		while (moduleStream.Position() < endPosition)
		{
			CV_DebugSSubsectionHeader_t subHeader;
			Assert(moduleStream.Read(&subHeader));
			Assert(subHeader.type == DEBUG_S_SUBSECTION_TYPE::DEBUG_S_LINES);
			
			CV_DebugSLinesHeader_t linesHeader;
			Assert(moduleStream.Read(&linesHeader));

			//Print("%04x:%08x-%08x\n", 1, linesHeader.offCon,
				//(linesHeader.offCon + linesHeader.cbCon));
			if (offset < linesHeader.offCon || offset >= (linesHeader.offCon + linesHeader.cbCon))
			{
				moduleStream.Skip(subHeader.cbLen - sizeof(CV_DebugSLinesHeader_t));
			}
			else
			{
				CV_DebugSLinesFileBlockHeader_t fileHeader;
				Assert(moduleStream.Read(&fileHeader));

				const uint32_t functionOffset = offset - linesHeader.offCon;

				CV_Line_t prev = { 0 };
				for (size_t i = 0; fileHeader.nLines; i++)
				{
					CV_Line_t line;
					Assert(moduleStream.Read(&line));

					if (functionOffset >= prev.offset && functionOffset < line.offset)
					{
						std::string& function = m_publics.GetFunction(linesHeader.offCon);

						Print("Function: %s Line: %d\n", function.c_str(), line.linenumStart);
						//Print("  Obj: %s\n", buffer);
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

	return true;
}

bool PdbDbi::GetSectionContribution(const uint32_t section, const uint32_t offset, SectionContribEntry2& entry)
{
	//Seek to start of section contribution stream
	m_stream.Seek(sizeof(DbiStreamHeader) + m_header.ModInfoSize);
	const uint32_t endPosition = m_stream.Position() + m_header.ModInfoSize + m_header.SectionContributionSize;

	//Reassert this here
	SectionContrSubstreamVersion version;
	Assert(m_stream.Read(&version));
	Assert(version == SectionContrSubstreamVersion::V2);

	while (m_stream.Position() < endPosition)
	{
		Assert(m_stream.Read<SectionContribEntry2>(&entry));

		if (entry.SC.Section != section)
			continue;

		if (entry.SC.Offset <= offset && (entry.SC.Offset + entry.SC.Size) > offset)
			return true;
	}
	
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
