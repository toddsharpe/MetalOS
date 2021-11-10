#include "string.h"
#include "stdlib.h"

LIBRARY char* strdup(char const* _String)
{
	char* dst = (char*)malloc(strlen(_String) + 1);
	if (dst == nullptr)
		return nullptr;

	strcpy(dst, _String);
	return dst;
}
