#include "stdio.h"
#include <MetalOS.h>

FILE* fopen(char const* _FileName, char const* _Mode)
{
	//inspect mode
	return CreateFile(_FileName, Read);
}
