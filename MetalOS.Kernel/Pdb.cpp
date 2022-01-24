#include <cstdint>
#include <windows/types.h>
#include <windows/winnt.h>
#include "Pdb.h"

Pdb::Pdb(const void* pdbAddress, const Handle image) :
	m_msfFile(pdbAddress),
	m_dbi(m_msfFile.GetStream(PDB_STREAM_DBI), m_msfFile, image),
	m_pdb(m_msfFile.GetStream(PDB_STREAM_PDB))
{
	//uint32_t index;
	//Assert(m_pdb.GetStream("/names", index));
	//Print("Names: 0x%x\n", index);

	//MsfStream& names = m_msfFile.GetStream(index);
	//names.Display();
}

bool Pdb::ResolveFunction(const uint32_t rva, PdbFunctionLookup& lookup)
{
	return m_dbi.ResolveFunction(rva, lookup);
}
