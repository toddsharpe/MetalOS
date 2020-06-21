#pragma once

#if defined(CRT_EXPORT)
#define LIBRARY __declspec(dllexport)
#elif defined(CRT_STATIC)
#define LIBRARY 
#else
#define LIBRARY __declspec(dllimport)
#endif
