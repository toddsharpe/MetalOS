#include "Kernel.h"
#include "Assert.h"

#include "PdbPublicsStream.h"

void PdbPublicsStream::Load(MsfStream& stream)
{
	//Load records
	//This has been found to have S_CONSTANT, S_UDT, S_GDATA32, S_LDATA32, S_LPROCREF, S_PUB32
	uint32_t offset = (1 << 31);
	while (stream.Position() < stream.GetSize())
	{
		RecordHeader header;
		Assert(stream.Read(&header));

		switch (header.Type)
		{
		case S_PUB32:
		case S_LPROCREF:
		case S_PROCREF:
			break;

		default:
		{
			//Printf("skipping 0x%x\n", header.Type);
			stream.Skip(header.Length - sizeof(header.Length));
			stream.SkipAlign<uint32_t>();
			continue;
		}
		}

		S_PUB32_Record record;
		Assert(stream.Read(&record));

		std::string name;
		Assert(stream.ReadString(name));

		if (header.Type != S_PUB32 && record.Section == Section::Text)
			m_functionLookup.insert({ record.Offset, name });

		stream.SkipAlign<uint32_t>();
	}
}

bool PdbPublicsStream::GetFunction(const uint32_t offset, std::string& name)
{
	const auto& it = m_functionLookup.find(offset);
	//AssertPrintInt(it != m_functionLookup.end(), offset);
	if (it == m_functionLookup.end())
	{
		name = "<not found>";
		return false;
	}
	else
	{
		name = it->second;
		return false;
	}
}
