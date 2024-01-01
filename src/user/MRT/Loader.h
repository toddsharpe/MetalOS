#pragma once

class Loader
{
public:
	static void* LoadLibrary(const char* const moduleName);
	static void ProcessImports(const void* const imageBase, const bool skipRT);
	static void CrtInit(const void* imageBase);
};
