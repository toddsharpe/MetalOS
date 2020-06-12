#pragma once
#include "msvc.h"
#include <cstdint>
#include "MsfFile.h"
#include "MsfStream.h"
#include "PdbDbi.h"
#include <string>
#include "MetalOS.Kernel.h"
#include "PdbPdb.h"

// https://github.com/willglynn/pdb
// https://llvm.org/docs/PDB/
// https://github.com/microsoft/microsoft-pdb
class Pdb
{
public:
	Pdb(uintptr_t address);
	
	bool PrintStack(const uint32_t rva);

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

