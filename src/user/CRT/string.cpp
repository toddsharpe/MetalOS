#pragma once

#include "core_crt/stdlib.h"
#include "core_crt/string.h"
#include "user/stdlib.h"
#include "user/string.h"

extern "C"
{
	char* strdup(char const* _String)
	{
		char* dst = (char*)malloc(strlen(_String) + 1);
		if (dst == nullptr)
			return nullptr;

		strcpy(dst, _String);
		return dst;
	}
}
