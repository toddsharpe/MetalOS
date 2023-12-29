#include "Kernel/Kernel.h"
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
			//Entries known how to parse
			case S_PUB32:
			case S_LPROCREF:
			case S_PROCREF:
				break;

			//Known entries to skip for now
			case S_CONSTANT:
			case S_UDT:
			case S_LDATA32:
			case S_GDATA32:
				stream.Skip(header.Length - sizeof(header.Length));
				stream.SkipAlign<uint32_t>();
				continue;

			//Bugcheck on unknown entries
			default:
			{
				Printf("Unknown record type: %d", header.Type);
				Assert(false);
			}
		}

		S_PUB32_Record record;
		Assert(stream.Read(&record));

		std::string name;
		Assert(stream.ReadString(name));

		if (record.Section == Section::Text)
			m_functionLookup.insert({ record.Offset, name });

		stream.SkipAlign<uint32_t>();
	}
}

bool PdbPublicsStream::GetFunction(const uint32_t offset, std::string& name)
{
	//Printf("GetFunction: 0x%x\n", offset);
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

void PdbPublicsStream::Display() const
{
	for (const auto& item : m_functionLookup)
	{
		Printf("0x%016x: %s\n", item.first, item.second.c_str());
	}
}
