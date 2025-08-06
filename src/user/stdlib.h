#pragma once

#include "core_crt/stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif

	void exit(int _Code);

	void* calloc(size_t _Count, size_t _Size);
	void* realloc(void* _Block, size_t _Size);
	void* malloc(size_t _Size);

	void free(void* _Block);

#ifdef __cplusplus
}
#endif