#pragma once
#include "msvc.h"
#include <cstdint>
#include <string>
#include "MsfFile.h"
#include "PdbDbi.h"
#include "PdbPdb.h"

// https://github.com/willglynn/pdb
// https://llvm.org/docs/PDB/
// https://github.com/microsoft/microsoft-pdb
// https://github.com/moyix/pdbparse/tree/master/src - undname from wine
class Pdb
{
public:
	Pdb(const void* pdbAddress, const Handle image);
	
	bool ResolveFunction(const uint32_t rva, PdbFunctionLookup& lookup);

private:
	enum Streams
	{
		PDB_STREAM_ROOT = 0, //# PDB root directory
		PDB_STREAM_PDB = 1, //# PDB stream info
		PDB_STREAM_TPI = 2, //# type info
		PDB_STREAM_DBI = 3, //# debug info
	};

	MsfFile m_msfFile;
	PdbDbi m_dbi;
	PdbPdb m_pdb;
};

