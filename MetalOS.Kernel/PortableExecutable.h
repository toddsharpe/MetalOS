#pragma once

#include "msvc.h"
#include <efi.h>
#include <LoaderParams.h>
#include <MetalOS.h>
#include <WindowsPE.h>
#include <cstdint>
#include "msvc.h"
#include <string>
#include "MetalOS.Kernel.h"

//https://deplinenoise.wordpress.com/2013/06/14/getting-your-pdb-name-from-a-running-executable-windows/
//https://gist.github.com/gimelfarb/8642282
class PortableExecutable
{
public:
	static size_t GetSizeOfImage(const Handle hModule);
	static PIMAGE_SECTION_HEADER GetPESection(const std::string& name, const uintptr_t ImageBase);
	static PIMAGE_SECTION_HEADER GetPESection(const uint32_t index, const uintptr_t ImageBase);
	static uintptr_t GetProcAddress(Handle hModule, const char* lpProcName);
	static const char* GetPdbName(Handle hModule);

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

