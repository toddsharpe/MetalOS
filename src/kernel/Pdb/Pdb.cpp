#include <cstdint>
#include <windows/types.h>
#include <windows/winnt.h>
#include "Pdb.h"

#include "Assert.h"

#include "user/MetalOS.Types.h"

Pdb::Pdb(const void* pdbAddress, const Handle image) :
	m_msfFile(pdbAddress),
	m_dbi(m_msfFile.GetStream(PDB_STREAM_DBI), m_msfFile, image),
	m_pdb(m_msfFile.GetStream(PDB_STREAM_PDB))
{
	
}

bool Pdb::ResolveFunction(const uint32_t rva, PdbFunctionLookup& lookup)
{
	return m_dbi.ResolveFunction(rva, lookup);
}
