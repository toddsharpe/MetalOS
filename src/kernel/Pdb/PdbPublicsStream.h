#pragma once

#include <cstdint>
#include "MsfStream.h"
#include <map>
#include <coreclr/cvinfo.h>

//Public stream is a list of records, concatenated, aligned on 4 bytes 
//RecordHeader
//Record
class PdbPublicsStream
{
public:
	void Load(MsfStream& stream);
	bool GetFunction(const uint32_t offset, std::string& name);

	void Display() const;

private:
	enum Section
	{
		Text = 1
	};

#pragma pack(push, 1)
	struct RecordHeader
	{
		uint16_t Length;
		SYM_ENUM_e Type;
	};

	struct S_PUB32_Record
	{
		uint32_t Flags;
		uint32_t Offset;
		uint16_t Section;
		char Name[];
	};
#pragma pack(pop)

	//For now, just store functions
	std::map<uint32_t, std::string> m_functionLookup;
};

