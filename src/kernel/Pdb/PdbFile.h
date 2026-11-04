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

	//False when the PDB isn't the image's (a stale PDB from another build would name the wrong functions), the image's headers don't lie in its imageSize bytes, or the PDB can't be read (not MSF, a header or stream it lacks or doesn't support, too large for the arena); it then resolves nothing
	bool Open(const void* pdb, const void* const image, const size_t imageSize, Arena& arena)
	{
		m_dbi = nullptr;
		m_msfFile = arena.Allocate<MsfFile>(pdb);
		if (!m_msfFile || !m_msfFile->Initialize(arena))
			return false;

		//Load msf file

		//The image's CodeView record names the PDB it was linked with; it's only found once the headers, and the section table the DBI reads, check out
		const WinPE::OMFSignatureRSDS* const codeView = WinPE::GetCodeView(image, imageSize);
		if (!codeView)
			return false;

		//Matched as the PDB library does: the same GUID, and an age the PDB stream has reached
		PdbStreamHeader header;
		if (!m_msfFile->HasStream(PDB_STREAM_PDB) || m_msfFile->GetStream(PDB_STREAM_PDB).Size < sizeof(header))
			return false;
		MsfStream& pdbStream = m_msfFile->GetStream(PDB_STREAM_PDB);
		Assert(pdbStream.Read(&header));
		if (memcmp(&header.Guid, &codeView->guid, sizeof(header.Guid)) != 0 || codeView->age > header.Age)
			return false;

		//DBI stream
		if (!m_msfFile->HasStream(PDB_STREAM_DBI))
			return false;
		MsfStream& dbiStream = m_msfFile->GetStream(PDB_STREAM_DBI);

		//Load DbiStream
		PdbDbi* const dbi = arena.Allocate<PdbDbi>(dbiStream, *m_msfFile);
		if (!dbi || !dbi->Initialize(image, arena))
			return false;

		//The DBI stream's age, when it has one, is the image's exactly
		if (dbi->GetAge() != 0 && dbi->GetAge() != codeView->age)
			return false;

		m_dbi = dbi;
		return true;
	}
	
	bool Resolve(const uint32_t rva, PdbLookup& lookup)
	{
		return m_dbi && m_dbi->ResolveFunction(rva, lookup);
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
