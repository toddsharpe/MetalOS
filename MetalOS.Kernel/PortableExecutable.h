#pragma once

#include "msvc.h"
#include <efi.h>
#include <LoaderParams.h>
#include <MetalOS.h>
#include <cstdint>
#include "msvc.h"
#include <string>
#include "MetalOS.Kernel.h"

//https://deplinenoise.wordpress.com/2013/06/14/getting-your-pdb-name-from-a-running-executable-windows/
//https://gist.github.com/gimelfarb/8642282
class PortableExecutable
{
public:
	static DWORD GetEntryPoint(const Handle hModule);
	static DWORD GetSizeOfImage(const Handle hModule);
	static PIMAGE_SECTION_HEADER GetPESection(const Handle hModule, const std::string& name);
	static PIMAGE_SECTION_HEADER GetPESection(const Handle hModule, const uint32_t index);
	static uintptr_t GetProcAddress(const Handle hModule, const char* lpProcName);
	static const char* GetPdbName(const Handle hModule);

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

