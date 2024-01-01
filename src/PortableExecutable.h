#pragma once

#include <windows/types.h>
#include <windows/winnt.h>
#include <cstdint>
#include <string>

//https://deplinenoise.wordpress.com/2013/06/14/getting-your-pdb-name-from-a-running-executable-windows/
//https://gist.github.com/gimelfarb/8642282

//This file is shared with kernel and MRT (which doesn't have a heap and therefore can't use new/malloc).
class PortableExecutable
{
public:
	static void* GetEntryPoint(const void* const imageBase);
	static uint32_t GetSizeOfImage(const void* const imageBase);

	static void* GetProcAddress(const void* const imageBase, const char* const procName);
	static const char* GetPdbName(const void* const imageBase);

	static const IMAGE_SECTION_HEADER* GetPESection(const void* const imageBase, const char* name);
	static const IMAGE_SECTION_HEADER* GetPESection(const void* const imageBase, const uint32_t index);

	static bool Contains(const void* const imageBase, const uintptr_t ip);

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

