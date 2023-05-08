#pragma once


#include <efi.h>
#include <kernel/LoaderParams.h>
#include <cstdint>

#include <string>
#include "MetalOS.Kernel.h"

//https://deplinenoise.wordpress.com/2013/06/14/getting-your-pdb-name-from-a-running-executable-windows/
//https://gist.github.com/gimelfarb/8642282
class PortableExecutable
{
public:
	static DWORD GetEntryPoint(void* const imageBase);
	static DWORD GetSizeOfImage(void* const imageBase);
	static PIMAGE_SECTION_HEADER GetPESection(void* const imageBase, const std::string& name);
	static PIMAGE_SECTION_HEADER GetPESection(void* const imageBase, const uint32_t index);
	static void* GetProcAddress(void* const imageBase, const std::string& procName);
	static const char* GetPdbName(void* const imageBase);

private:
	//GuidDef.h
	typedef struct {
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	} GUID;

	//ReactOS::sdk\include\reactos\wine\mscvpdb.h
	typedef struct OMFSignatureRSDS
	{
		char        Signature[4];
		GUID        guid;
		DWORD       age;
		CHAR        name[1];
	} OMFSignatureRSDS;
};

