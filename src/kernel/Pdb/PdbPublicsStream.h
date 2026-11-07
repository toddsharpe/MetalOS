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

	//False when the arena can't hold the table and its names (a PDB too large for it)
	bool Load(MsfStream& stream, Arena& arena)
	{
		//Count the .text publics first, so the table is sized to hold every one of them
		const size_t count = Scan(stream, nullptr, arena);
		m_functions = static_cast<Function*>(arena.Allocate(count * sizeof(Function)));
		if (!m_functions)
			return false;

		stream.Seek(0);
		m_count = Scan(stream, m_functions, arena);
		return m_count == count;
	}

	bool GetFunction(const uint32_t offset, cstr_t& name)
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
			Printf("0x%016x: %.*s\n", m_functions[i].Offset, (int)m_functions[i].Name.Size, m_functions[i].Name.Data);
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
		cstr_t Name; //Copied into the arena, since a name can straddle two MSF blocks
	};

	//Returns the number of .text publics, and stores each one (name copied into the arena) when given a table, stopping at the first name the arena can't hold
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

					char name[256];
					Assert(stream.ReadString({ name, sizeof(name) }));
					const size_t length = StrLength(name);

					if (record.Section == Section::Text)
					{
						if (table)
						{
							//Copied by hand, so a full arena fails the load, which Arena::Copy can't report
							char* const copy = static_cast<char*>(arena.Allocate(length + 1));
							if (!copy)
								return count;
							memcpy(copy, name, length);
							table[count] = { record.Offset, { copy, length } };
						}
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
	};
#pragma pack(pop)

	Function* m_functions;
	size_t m_count;
};

