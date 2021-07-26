#include "Kernel.h"
#include "Assert.h"

#include "PdbPublicsStream.h"

void PdbPublicsStream::Load(MsfStream& stream)
{
	//Load records
	while (stream.Position() < stream.GetSize())
	{
		RecordHeader header;
		Assert(stream.Read(&header));
		Assert(header.Type == SYM_ENUM_e::S_PUB32);

		S_PUB32_Record record;
		Assert(stream.Read(&record));

		std::string name;
		Assert(stream.ReadString(name));

		if (record.Section == Section::Text)
			m_functionLookup.insert({ record.Offset, name });

		stream.SkipAlign<uint32_t>();
	}
}

std::string& PdbPublicsStream::GetFunction(const uint32_t offset)
{
	const auto& it = m_functionLookup.find(offset);
	Assert(it != m_functionLookup.end());

	return it->second;
}
