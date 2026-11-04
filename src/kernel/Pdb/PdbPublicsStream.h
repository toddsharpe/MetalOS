#pragma once

#include "kernel/Pdb/MsfStream.h"
#include "kernel/Pdb/Pdb.h"
#include "Assert.h"

//Public stream is a list of records, concatenated, aligned on 4 bytes 
//RecordHeader
//Record

//This is actually the SymRecordStream
class PdbPublicsStream
{
public:
	PdbPublicsStream() : m_functions(), m_count()
	{

	}

	void Load(MsfStream& stream, Arena& arena)
	{
		//Count the .text publics first, so the table is sized to hold every one of them
		m_count = Scan(stream, nullptr, arena);
		m_functions = static_cast<Function*>(arena.Allocate(m_count * sizeof(Function)));
		Assert(m_functions);

		stream.Seek(0);
		Scan(stream, m_functions, arena);
	}

	bool GetFunction(const uint32_t offset, CString& name)
	{
		for (size_t i = 0; i < m_count; i++)
		{
			if (m_functions[i].Offset != offset)
				continue;

			name = m_functions[i].Name;
			return true;
		}
		return false;
	}

	void Display() const
	{
		for (size_t i = 0; i < m_count; i++)
		{
			Printf("0x%016x: %s\n", m_functions[i].Offset, m_functions[i].Name.c_str());
		}
	}

private:
	enum Section
	{
		Text = 1
	};

	struct Function
	{
		uint32_t Offset;
		CString Name; //Copied into the arena, since a name can straddle two MSF blocks
	};

	//Returns the number of .text publics, and stores each one (name copied into the arena) when given a table
	size_t Scan(MsfStream& stream, Function* const table, Arena& arena)
	{
		size_t count = 0;

		//Records seen so far: S_CONSTANT, S_UDT, S_GDATA32, S_LDATA32, S_LPROCREF, S_PROCREF, S_PUB32
		while (stream.Position() < stream.Size)
		{
			RecordHeader header;
			Assert(stream.Read(&header));

			//A record too short to hold its own type would never advance the walk
			if (header.Length < sizeof(header.Type))
				break;

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
						if (table)
							table[count] = { record.Offset, arena.Copy(name) };
						count++;
					}
				}
				break;

				//Everything else is skipped by its length, including types not seen before (S_LTHREAD32 and S_GTHREAD32 come with thread_local)
				default:
					stream.Skip(header.Length - sizeof(header.Length));
					continue;
			}

			stream.SkipAlign<uint32_t>();
		}

		return count;
	}

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

	Function* m_functions;
	size_t m_count;
};

