#pragma once

#include <crt_stdio.h>

typedef void* FILE;

LIBRARY FILE* fopen(char const* _FileName, char const* _Mode);

