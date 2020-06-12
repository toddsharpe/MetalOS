#include "PdbPdb.h"
#include "Main.h"

PdbPdb::PdbPdb(MsfStream& stream) :
	m_stream(stream),
	m_namesBuffer(),
	m_table()
{
	PdbStreamHeader header;
	Assert(m_stream.Read(&header));
	Assert(header.Version == PdbStreamVersion::VC70);

	m_namesBuffer.Length = m_stream.Read<uint32_t>();
	Print("0x%x\n", m_namesBuffer.Length);
	m_namesBuffer.Data = malloc(m_namesBuffer.Length);
	m_stream.Read((char*)m_namesBuffer.Data, m_namesBuffer.Length);
	m_table.Load(m_stream);
}

bool PdbPdb::GetStream(const std::string& name, uint32_t& index)
{
	char* pointer = (char*)m_namesBuffer.Data;
	const char* stop = (char*)m_namesBuffer.Data + m_namesBuffer.Length;
	kernel.PrintBytes((char*)m_namesBuffer.Data, m_namesBuffer.Length);
	while (strcmp(name.c_str(), pointer) != 0)
	{
		pointer += strlen(pointer) + 1;

		if (pointer >= stop)
			return false;
	}

	const uint32_t key = (uintptr_t)pointer - (uintptr_t)m_namesBuffer.Data;
	return m_table.GetValue(key, index);
}
