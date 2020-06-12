#include "Pdb.h"
#include <cstdint>
#include "Main.h"

Pdb::Pdb(uintptr_t address) :
	m_msfFile(address),
	m_dbi(m_msfFile.GetStream(PDB_STREAM_DBI), m_msfFile),
	m_pdb(m_msfFile.GetStream(PDB_STREAM_PDB))
{
	//uint32_t index;
	//Assert(m_pdb.GetStream("/names", index));
	//Print("Names: 0x%x\n", index);

	//MsfStream& names = m_msfFile.GetStream(index);
	//names.Display();
}

bool Pdb::PrintStack(const uint32_t rva)
{
	return m_dbi.PrintStack(rva);
}
