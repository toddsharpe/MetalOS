#pragma once
#include "msvc.h"
#include "MsfStream.h"
#include "MicrosoftHyperV.h"
#include "PdbHashTable.h"
#include "MetalOS.Kernel.h"
#include <string>

//https://llvm.org/docs/PDB/PdbStream.html
//Header
//Length
//Names Buffer
//HashTable
//4 bytes of zero?!
//Feature codes
class PdbPdb
{
public:
	PdbPdb(MsfStream& stream);

	bool GetStream(const std::string& name, uint32_t& index);

private:
	enum class PdbStreamVersion : uint32_t
	{
		VC2 = 19941610,
		VC4 = 19950623,
		VC41 = 19950814,
		VC50 = 19960307,
		VC98 = 19970604,
		VC70Dep = 19990604,
		VC70 = 20000404,
		VC80 = 20030901,
		VC110 = 20091201,
		VC140 = 20140508,
	};
	
#pragma pack(push, 1)
	struct PdbStreamHeader
	{
		PdbStreamVersion Version;
		uint32_t Signature;
		uint32_t Age;
		guid_t UniqueId;
	};
#pragma pack(pop)


	MsfStream& m_stream;
	Buffer m_namesBuffer;
	PdbHashTable m_table;
};

