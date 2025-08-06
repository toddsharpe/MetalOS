#pragma once

#include "kernel/Types.h"
#include "kernel/Pdb/MsfFile.h"
#include "kernel/Pdb/PdbDbi.h"
//#include "kernel/Pdb/PdbPdb.h"
#include "kernel/Arch.h"
#include "kernel/Pdb/MsfFile.h"
#include "kernel/Pdb/MsfStream.h"

// https://github.com/willglynn/pdb
// https://llvm.org/docs/PDB/
// https://github.com/microsoft/microsoft-pdb
// https://github.com/moyix/pdbparse/tree/master/src - undname from wine

class PdbFile
{
public:
	PdbFile() : m_msfFile()//, m_dbi(), m_pdb()
	{
		
	}

	void Open(const void* pdb, const void* const image, Arena& arena)
	{
		m_msfFile = arena.Allocate<MsfFile>(pdb);
		Assert(m_msfFile);
		m_msfFile->Initialize(arena);

		//Load msf file

		//DBI stream
		MsfStream& dbiStream = m_msfFile->GetStream(PDB_STREAM_DBI);
		MsfStream& pdbStream = m_msfFile->GetStream(PDB_STREAM_PDB);

		//Load DbiStream
		m_dbi = arena.Allocate<PdbDbi>(dbiStream, *m_msfFile);
		Assert(m_dbi);
		m_dbi->Initialize(image, arena);

		//Load PdbStream
		//void* pdbAlloc = arena.Allocate(sizeof(PdbPdb));
		//Assert(pdbAlloc);
		//m_pdb = new (pdbAlloc) PdbPdb(pdbStream);
	}
	
	bool Resolve(const uint32_t rva, PdbLookup& lookup)
	{
		return m_dbi->ResolveFunction(rva, lookup);
	}

private:
	static constexpr size_t ArenaSize = PageSize << 10; // 4MB Kernel Arena

	enum Streams
	{
		PDB_STREAM_ROOT = 0, //# PDB root directory
		PDB_STREAM_PDB = 1, //# PDB stream info
		PDB_STREAM_TPI = 2, //# type info
		PDB_STREAM_DBI = 3, //# debug info
	};

	MsfFile* m_msfFile;
	PdbDbi* m_dbi;
	//PdbPdb* m_pdb;
};
