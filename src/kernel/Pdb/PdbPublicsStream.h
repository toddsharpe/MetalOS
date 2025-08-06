#pragma once

#include "kernel/Pdb/MsfStream.h"
#include "kernel/Pdb/Pdb.h"
#include "Lib/StaticMap.h"
#include "Assert.h"

//Public stream is a list of records, concatenated, aligned on 4 bytes 
//RecordHeader
//Record

//This is actually the SymRecordStream
class PdbPublicsStream
{
public:
	PdbPublicsStream() : m_functionLookup()
	{

	}

	void Load(MsfStream& stream, Arena& arena)
	{
		//Load records
		//This has been found to have S_CONSTANT, S_UDT, S_GDATA32, S_LDATA32, S_LPROCREF, S_PUB32
		while (stream.Position() < stream.Size)
		{
			RecordHeader header;
			Assert(stream.Read(&header));

			switch (header.Type)
			{
				case S_PUB32:
				{
					S_PUB32_Record record;
					Assert(stream.Read(&record));

					StaticString<256> name;
					Assert(stream.ReadString(name));

					if (record.Section == Section::Text)
					{
						String copied = arena.Copy(name);
						m_functionLookup.Add(record.Offset, copied);
					}
				}
				break;

				//Known entries to skip for now
				case S_LPROCREF:
				case S_PROCREF:
				case S_CONSTANT:
				case S_UDT:
				case S_LDATA32:
				case S_GDATA32:
					stream.Skip(header.Length - sizeof(header.Length));
					continue;

				//Bugcheck on unknown entries
				default:
				{
					Printf("Unknown record type: %d", header.Type);
					Assert(false);
				}
			}

			stream.SkipAlign<uint32_t>();
		}
	}

	bool GetFunction(const uint32_t offset, CString& name)
	{
		return m_functionLookup.Get(offset, name);
	}

	void Display() const
	{
		for (const auto& item : m_functionLookup)
		{
			Printf("0x%016x: %s\n", item.Key, item.Value.c_str());
		}
	}

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

	//Containers pointer into loaded PDB image
	StaticMap<uint32_t, CString, 4*1024> m_functionLookup;
};

