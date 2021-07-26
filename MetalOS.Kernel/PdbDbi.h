#pragma once

#include <windows/types.h>
#include <windows/winnt.h>
#include <microsoft-pdb/cvinfo.h>

#include "MsfStream.h"
#include "PdbPublicsStream.h"

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
	PdbDbi(MsfStream& stream, MsfFile& file);

	bool PrintStack(const uint32_t rva);

private:
	static const uint32_t TextSection = 1;
	static const uint32_t Signature = -1;

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
		int32_t Offset;
		int32_t Size;
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


	enum DEBUG_S_SUBSECTION_TYPE {
		DEBUG_S_IGNORE = 0x80000000,    // if this bit is set in a subsection type then ignore the subsection contents

		DEBUG_S_SYMBOLS = 0xf1,
		DEBUG_S_LINES,
		DEBUG_S_STRINGTABLE,
		DEBUG_S_FILECHKSMS,
		DEBUG_S_FRAMEDATA,
		DEBUG_S_INLINEELINES,
		DEBUG_S_CROSSSCOPEIMPORTS,
		DEBUG_S_CROSSSCOPEEXPORTS,

		DEBUG_S_IL_LINES,
		DEBUG_S_FUNC_MDTOKEN_MAP,
		DEBUG_S_TYPE_MDTOKEN_MAP,
		DEBUG_S_MERGED_ASSEMBLYINPUT,

		DEBUG_S_COFF_SYMBOL_RVA,
	};

	typedef          long   CV_off32_t;

	struct CV_DebugSSubsectionHeader_t
	{
		enum DEBUG_S_SUBSECTION_TYPE type;
		CV_off32_t                   cbLen; //Size of record following
	};

	//Ex
	//  C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.26.28801\include\xtree
	// (MD5: 0A759C57860302D8664203C1D6CFE56F),
	// 0001:00010330-00010411, - segCon:offCon-offCon+cbCon
	// line/addr pairs = 8 - nLines

	struct CV_DebugSLinesHeader_t {
		CV_off32_t     offCon; //start of offset
		unsigned short segCon;
		unsigned short flags;
		CV_off32_t     cbCon;
	};

	struct CV_Line_t
	{
		uint32_t offset;             // Offset to start of code bytes for line number
		uint32_t linenumStart : 24;    // line where statement/expression starts
		uint32_t deltaLineEnd : 7;     // delta to line where statement ends (optional)
		uint32_t fStatement : 1;       // true if a statement linenumber, else an expression line num
	};

	struct CV_DebugSLinesFileBlockHeader_t {
		CV_off32_t     offFile; //offset into name stream?
		CV_off32_t     nLines;
		CV_off32_t     cbBlock; //Not sure what this is
		// CV_Line_t      lines[nLines];
		// CV_Column_t    columns[nColumns]; - no columns
	};

	//
	// Line flags (data present)
	//
#define CV_LINES_HAVE_COLUMNS 0x0001

	typedef unsigned short CV_columnpos_t;    // byte offset in a source line

	struct CV_Column_t {
		CV_columnpos_t offColumnStart;
		CV_columnpos_t offColumnEnd;
	};

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

	bool GetSectionContribution(const uint32_t section, const uint32_t offset, SectionContribEntry2& entry);
	bool GetModuleInfo(const uint32_t index, ModInfo& info);

	MsfStream& m_stream;
	MsfFile& m_file;
	PdbPublicsStream m_publics;
	DbiStreamHeader m_header;
	PIMAGE_SECTION_HEADER m_kernelText;
};

