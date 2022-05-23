#include <stdlib.h>
#include <string.h>

char* strdup(char const* _String)
{
	char* dst = (char*)malloc(strlen(_String) + 1);
	if (dst == nullptr)
		return nullptr;

	strcpy(dst, _String);
	return dst;
}
