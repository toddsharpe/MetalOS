#pragma once

#include "kernel/Types.h"
#include "kernel/Pdb/MsfFile.h"
#include "kernel/Pdb/PdbDbi.h"
#include "kernel/Pdb/MsfStream.h"

// https://github.com/willglynn/pdb
// https://llvm.org/docs/PDB/
// https://github.com/microsoft/microsoft-pdb
// https://github.com/moyix/pdbparse/tree/master/src - undname from wine

class PdbFile
{
public:
	PdbFile() : m_msfFile(), m_dbi()
	{
		
	}

	//False when the PDB isn't the image's (a stale PDB from another build would name the wrong functions)
	bool Open(const void* pdb, const void* const image, Arena& arena)
	{
		m_msfFile = arena.Allocate<MsfFile>(pdb);
		Assert(m_msfFile);
		m_msfFile->Initialize(arena);

		//Load msf file

		//The image's CodeView record names the PDB it was linked with
		const WinPE::OMFSignatureRSDS* const codeView = WinPE::GetCodeView(image);
		if (!codeView)
			return false;

		//Matched as the PDB library does: the same GUID, and an age the PDB stream has reached
		PdbStreamHeader header;
		MsfStream& pdbStream = m_msfFile->GetStream(PDB_STREAM_PDB);
		Assert(pdbStream.Read(&header));
		if (memcmp(&header.Guid, &codeView->guid, sizeof(header.Guid)) != 0 || codeView->age > header.Age)
			return false;

		//DBI stream
		MsfStream& dbiStream = m_msfFile->GetStream(PDB_STREAM_DBI);

		//Load DbiStream
		m_dbi = arena.Allocate<PdbDbi>(dbiStream, *m_msfFile);
		Assert(m_dbi);
		m_dbi->Initialize(image, arena);

		//The DBI stream's age, when it has one, is the image's exactly
		return m_dbi->GetAge() == 0 || m_dbi->GetAge() == codeView->age;
	}
	
	bool Resolve(const uint32_t rva, PdbLookup& lookup)
	{
		return m_dbi->ResolveFunction(rva, lookup);
	}

private:
	enum Streams
	{
		PDB_STREAM_ROOT = 0, //# PDB root directory
		PDB_STREAM_PDB = 1, //# PDB stream info
		PDB_STREAM_TPI = 2, //# type info
		PDB_STREAM_DBI = 3, //# debug info
	};

	//https://llvm.org/docs/PDB/PdbStream.html
	struct PdbStreamHeader
	{
		uint32_t Version;
		uint32_t Signature;
		uint32_t Age;
		WinPE::GUID Guid;
	};
	static_assert(sizeof(PdbStreamHeader) == 28, "Invalid PdbStreamHeader");

	MsfFile* m_msfFile;
	PdbDbi* m_dbi;
};
