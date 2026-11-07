#pragma once

#include "MsfStream.h"
#include "PdbPublicsStream.h"
#include "WinPE.h"

//https://github.com/Microsoft/microsoft-pdb/blob/master/PDB/dbi/dbi.h
//https://llvm.org/docs/PDB/DbiStream.html
//Stream Schematic:
//DBI Header - 64 bytes
//Variable length streams (7):
//  ModInfo
//  SectionContribution
//  SectionMap
//  SourceInfo
//  TypeServerMap
//  OptionalDbgHeader
//  ECSubstream
class PdbDbi
{
public:
	PdbDbi(MsfStream& stream, MsfFile& file) : 
		m_stream(stream),
		m_file(file),
		m_publics(),
		m_header(),
		m_textSection()
	{

	}

	//False for a DBI stream this doesn't read (too short, another version or machine, no symbol records), an image whose first section isn't .text, or a full arena
	bool Initialize(const void* const image, Arena& arena)
	{
		if (m_stream.Size < sizeof(m_header))
			return false;
		Assert(m_stream.Read(&m_header));
		if (m_header.VersionSignature != -1 || m_header.VersionHeader != DbiStreamVersion::V70 || m_header.Machine != 0x8664)
			return false;

		//The PDB numbers sections from 1 and lines are only looked up in section 1, which has to be the image's .text
		if (WinPE::GetNtHeaders(image)->FileHeader.NumberOfSections < TextSection)
			return false;
		m_textSection = WinPE::GetPESection(image, TextSection - 1);
		if (memcmp(m_textSection->Name, ".text", sizeof(".text")) != 0)
			return false;

		if (!m_file.HasStream(m_header.SymRecordStreamIndex))
			return false;
		MsfStream stream = m_file.GetStream(m_header.SymRecordStreamIndex);
		return m_publics.Load(stream, arena);
	}

	uint32_t GetAge() const
	{
		return m_header.Age;
	}

	bool ResolveFunction(const uint32_t rva, PdbLookup& lookup)
	{
		const uint32_t offset = rva - m_textSection->VirtualAddress;
		//Printf("Text VA: 0x%016x, Off: 0x%016x\n", m_textSection->VirtualAddress, offset);

		//No module contributes the IP when it's outside .text (a call into data) or in padding between contributions
		uint16_t moduleIndex;
		if (!GetModuleIndex(TextSection, offset, moduleIndex))
			return false;
		//Printf("ModuleIndex: %d\n", moduleIndex);

		ModInfo info;
		if (!GetModuleInfo(moduleIndex, info))
			return false;
		if (info.C13ByteSize == 0)
		{
			//TODO(tsharpe): Find the entry somewhere
			StrCopy(lookup.Function, Str("<not found1>"));
			lookup.Line = 0;
			return false;
		}

		//Module and object names follow; skipped rather than copied, since object paths can be arbitrarily long
		m_stream.SkipString();
		m_stream.SkipString();

		//A module whose stream the directory doesn't have (0xFFFF for none) has no lines to read
		if (!m_file.HasStream(info.ModuleSymStream))
			return false;
		MsfStream& moduleStream = m_file.GetStream(info.ModuleSymStream);
		if (moduleStream.Read<uint32_t>() != SymbolsSignatureC13)
			return false;

		//Older C11 line info, when a module has any, sits between the symbols and the C13 lines
		const uint32_t startPosition = info.SymByteSize + info.C11ByteSize;
		moduleStream.Seek(startPosition);

		//After seeking past the symbols, PDB contains records in order:
		// * CV_DebugSLinesHeader_t (DEBUG_S_LINES) (or DEBUG_S_FILECHKSMS, not handled here)
		// * CV_DebugSLinesHeader_t - defines range of offsets in segment
		// * CV_DebugSLinesFileBlockHeader_t - defines number of lines
		// * CV_Line_t - defines each line entry
		{
			//A function's lines are one subsection, and a module's subsections aren't sorted by offset (dynamic initializers come last), so each is checked
			const uint32_t endPosition = startPosition + info.C13ByteSize;
			while (moduleStream.Position() < endPosition)
			{
				CV_DebugSSubsectionHeader_t sectionHeader;
				Assert(moduleStream.Read(&sectionHeader));

				//Each subsection is left by seeking to the next (4-byte aligned) one, so an unmatched lines block is never read as headers
				const uint32_t sectionStart = moduleStream.Position();
				if (sectionHeader.cbLen < 0 || sectionStart > endPosition || (uint32_t)sectionHeader.cbLen > endPosition - sectionStart)
					return false;
				const uint32_t nextSection = AlignSize(sectionStart + (uint32_t)sectionHeader.cbLen, sizeof(uint32_t));

				if (sectionHeader.type != DEBUG_S_SUBSECTION_TYPE::DEBUG_S_LINES)
				{
					moduleStream.Seek(nextSection);
					continue;
				}

				CV_DebugSLinesHeader_t linesHeader;
				Assert(moduleStream.Read(&linesHeader));

				//Printf("%04x:%08x-%08x\n", 1, linesHeader.offCon, (linesHeader.offCon + linesHeader.cbCon));

				//Another function's lines (or another section's): continue to next entry
				if (linesHeader.segCon != TextSection || offset < (uint32_t)linesHeader.offCon || offset - (uint32_t)linesHeader.offCon >= (uint32_t)linesHeader.cbCon)
				{
					moduleStream.Seek(nextSection);
					continue;
				}

				//The IP's line is the one starting closest before it, in any file block (a function's code can come from more than one file)
				const uint32_t targetOffset = offset - linesHeader.offCon;
				bool found = false;
				CV_Line_t closest = {};
				while (moduleStream.Position() + sizeof(CV_DebugSLinesFileBlockHeader_t) <= nextSection)
				{
					const uint32_t blockStart = moduleStream.Position();
					CV_DebugSLinesFileBlockHeader_t fileHeader;
					Assert(moduleStream.Read(&fileHeader));

					//Lines (then columns, if the subsection has them) fill the rest of the block
					const uint32_t blockSize = (uint32_t)fileHeader.cbBlock;
					if (fileHeader.nLines < 0 || blockSize < sizeof(fileHeader) || blockSize > nextSection - blockStart || (uint64_t)fileHeader.nLines * sizeof(CV_Line_t) > blockSize - sizeof(fileHeader))
						return false;

					for (size_t i = 0; i < (size_t)fileHeader.nLines; i++)
					{
						CV_Line_t line;
						Assert(moduleStream.Read(&line));

						if (line.offset <= targetOffset && (!found || line.offset >= closest.offset))
						{
							closest = line;
							found = true;
						}
					}
					moduleStream.Seek(blockStart + blockSize);
				}

				//Code ahead of the function's first line, such as a prolog not attributed to a line
				if (!found)
					return false;

				cstr_t name;
				if (m_publics.GetFunction(linesHeader.offCon, name))
					StrCopy(lookup.Function, name);
				else if (!GetProcedureName(moduleStream, info.SymByteSize, offset, { lookup.Function, sizeof(lookup.Function) }))
					StrCopy(lookup.Function, Str("<not found>"));
				lookup.Line = closest.linenumStart;
				return true;
			}
		}

		StrCopy(lookup.Function, Str("<not found2>"));
		lookup.Line = 0;
		return false;
	}

private:
	static constexpr uint32_t TextSection = 1;
	static constexpr uint32_t Signature = 0xFFFFFFFF;
	static constexpr uint32_t SymbolsSignatureC13 = 4; //CV_SIGNATURE_C13, the first dword of a module's symbol stream

	enum class DbiStreamVersion : uint32_t
	{
		VC41 = 930803,
		V50 = 19960307,
		V60 = 19970606,
		V70 = 19990903,
		V110 = 20091201
	};

	struct DbiStreamHeader
	{
		int32_t VersionSignature; //-1
		DbiStreamVersion VersionHeader; //V70 
		uint32_t Age;
		uint16_t GlobalStreamIndex;
		struct
		{
			uint16_t MinorVersion : 8;
			uint16_t MajorVersion : 7;
			uint16_t NewVersionFormat : 1;
		} BuildNumber;
		uint16_t PublicStreamIndex;
		uint16_t PdbDllVersion;
		uint16_t SymRecordStreamIndex;
		uint16_t PdbDllRbld;
		int32_t ModInfoSize;
		int32_t SectionContributionSize;
		int32_t SectionMapSize;
		int32_t SourceInfoSize;
		int32_t TypeServerMapSize;
		uint32_t MFCTypeServerIndex;
		int32_t OptionalDbgHeaderSize;
		int32_t ECSubstreamSize;
		struct
		{
			uint16_t WasIncrementallyLinked : 1;
			uint16_t ArePrivateSymbolsStripped : 1;
			uint16_t HasConflictingTypes : 1;
			uint16_t Reserved : 13;
		} Flags;
		uint16_t Machine; //0x8664
		uint32_t Padding;
	};
	static_assert(sizeof(DbiStreamHeader) == 64, "Invalid DbiHeader");

	struct SectionContribEntry
	{
		uint16_t Section;
		char Padding1[2];
		uint32_t Offset;
		uint32_t Size;
		uint32_t Characteristics;
		uint16_t ModuleIndex;
		char Padding2[2];
		uint32_t DataCrc;
		uint32_t RelocCrc;
	};

	//struct MODI
	struct ModInfo
	{
		uint32_t Unused1;
		SectionContribEntry SectionContr;
		uint16_t Flags; //-1 means no debug info present (stripped)
		uint16_t ModuleSymStream;
		uint32_t SymByteSize; //Includes 4 byte header
		uint32_t C11ByteSize;
		uint32_t C13ByteSize;//Modern
		uint16_t SourceFileCount;
		char Padding[2];
		uint32_t Unused2;
		uint32_t SourceFileNameIndex;
		uint32_t PdbFilePathNameIndex;
		//char ModuleName[];
		//char ObjFileName[];
	};

#pragma pack(push, 1)
	struct SymbolHeader
	{
		uint16_t Length; //Of the record after this field
		SYM_ENUM_e Type;
	};

	//PROCSYM32, the body of S_GPROC32, S_LPROC32 and their _ID forms
	struct ProcedureSymbol
	{
		uint32_t Parent;
		uint32_t End; //Module stream offset of its S_END
		uint32_t Next;
		uint32_t Length;
		uint32_t DebugStart;
		uint32_t DebugEnd;
		uint32_t TypeIndex;
		uint32_t Offset;
		uint16_t Segment;
		uint8_t Flags;
		//char Name[];
	};
#pragma pack(pop)
	static_assert(sizeof(ProcedureSymbol) == 35, "Invalid ProcedureSymbol");

	enum class SectionContrSubstreamVersion : uint32_t
	{
		Ver60 = 0xeffe0000 + 19970605,
		V2 = 0xeffe0000 + 20140516
	};

	//V2
	struct SectionContribEntry2
	{
		SectionContribEntry SC;
		uint32_t ISectCoff;
	};

	struct SectionMapHeader
	{
		uint16_t Count;    // Number of segment descriptors
		uint16_t LogCount; // Number of logical segment descriptors
	};

	struct SectionMapEntry
	{
		uint16_t Flags;         // See the SectionMapEntryFlags enum below.
		uint16_t Ovl;           // Logical overlay number
		uint16_t Group;         // Group index into descriptor array.
		uint16_t Frame;
		uint16_t SectionName;   // Byte index of segment / group name in string table, or 0xFFFF.
		uint16_t ClassName;     // Byte index of class in string table, or 0xFFFF.
		uint32_t Offset;        // Byte offset of the logical segment within physical segment.  If group is set in flags, this is the offset of the group.
		uint32_t SectionLength; // Byte count of the segment or group.
	};

	enum class SectionMapEntryFlags : uint16_t
	{
		Read = 1 << 0,              // Segment is readable.
		Write = 1 << 1,             // Segment is writable.
		Execute = 1 << 2,           // Segment is executable.
		AddressIs32Bit = 1 << 3,    // Descriptor describes a 32-bit linear address.
		IsSelector = 1 << 8,        // Frame represents a selector.
		IsAbsoluteAddress = 1 << 9, // Frame represents an absolute address.
		IsGroup = 1 << 10           // If set, descriptor represents a group.
	};


	//Ex CV_DebugSLinesHeader_t
	//  C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.26.28801\include\xtree
	// (MD5: 0A759C57860302D8664203C1D6CFE56F),
	// 0001:00010330-00010411, - segCon:offCon-offCon+cbCon
	// line/addr pairs = 8 - nLines

	//Module-specific stream
	//struct ModiStream
	//{
	//	uint32_t Signature;
	//	uint8_t Symbols[SymByteSize - 4];
	//	uint8_t C11LineInfo[C11Size]; //0
	//	uint8_t C13LineInfo[C13Size];

	//	uint32_t GlobalRefsSize; //0
	//	uint8_t GlobalRefs[GlobalRefsSize]; //0
	//};

	bool GetModuleIndex(const uint32_t section, const uint32_t offset, uint16_t& moduleIndex)
	{
		//Seek to start of section contribution stream
		m_stream.Seek(sizeof(DbiStreamHeader) + m_header.ModInfoSize);
		const uint32_t endPosition = m_stream.Position() + m_header.SectionContributionSize;

		//Reassert this here
		SectionContrSubstreamVersion version;
		Assert(m_stream.Read(&version));
		if (version != SectionContrSubstreamVersion::Ver60)
			return false; //V2 uses SectionContribEntry2
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

		return false;
	}

	bool GetModuleInfo(const uint32_t index, ModInfo& info)
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

	//A static or anonymous-namespace function has no public (and an assembly one's lines can start before its public), so it's named by the procedure record holding offset
	bool GetProcedureName(MsfStream& moduleStream, const uint32_t symbolsSize, const uint32_t offset, const str_t name)
	{
		const uint32_t symbolsEnd = Min(symbolsSize, moduleStream.Size); //A SymByteSize past the stream's end would read past its blocks

		//Records follow the stream's signature, each one starting where the last one's length ends
		uint32_t position = sizeof(uint32_t);
		while (position < symbolsEnd && symbolsEnd - position >= sizeof(SymbolHeader))
		{
			moduleStream.Seek(position);
			SymbolHeader header;
			Assert(moduleStream.Read(&header));

			//A record too short for its type, or running past the symbols, is corrupt
			const uint32_t next = position + sizeof(header.Length) + header.Length;
			if (header.Length < sizeof(header.Type) || next > symbolsEnd)
				return false;

			const bool isProcedure = header.Type == S_GPROC32 || header.Type == S_LPROC32 || header.Type == S_GPROC32_ID || header.Type == S_LPROC32_ID;
			if (!isProcedure || header.Length < sizeof(header.Type) + sizeof(ProcedureSymbol))
			{
				position = next;
				continue;
			}

			ProcedureSymbol procedure;
			Assert(moduleStream.Read(&procedure));
			if (procedure.Segment == TextSection && offset - procedure.Offset < procedure.Length)
			{
				//The name is NUL-terminated inside the record; one longer than the destination is clipped
				size_t count = 0;
				name[0] = '\0';
				while (moduleStream.Position() < next)
				{
					const char c = moduleStream.Read<char>();
					if (c == '\0')
						return true;
					if (count < name.Size - 1)
					{
						name[count++] = c;
						name[count] = '\0';
					}
				}
				return false;
			}

			//Its locals, blocks and inlinees come before its S_END, so the walk jumps there (or to the next record, if End is out of place)
			position = (procedure.End > position && procedure.End < symbolsEnd) ? procedure.End : next;
		}

		return false;
	}

	MsfStream& m_stream;
	MsfFile& m_file;
	PdbPublicsStream m_publics;
	DbiStreamHeader m_header;
	const IMAGE_SECTION_HEADER* m_textSection;
};

