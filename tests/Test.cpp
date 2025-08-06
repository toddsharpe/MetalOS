#include <cstdio>
#include <cstdlib>
#include <cstdarg>

//Libs
#include "core_crt/string.c"

//Tests
#include "Lib/Arena_test.cpp"
#include "Lib/Buffer_test.cpp"
#include "Lib/String_test.cpp"
#include "Lib/StaticStack_test.cpp"
#include "Lib/CircularBuffer_test.cpp"
#include "Lib/Bitset_test.cpp"
#include "Lib/StaticVector_test.cpp"
#include "Lib/StaticMap_test.cpp"
#include "Lib/ObjectPool_test.cpp"
#include "Lib/RingBuffer_test.cpp"

#define Test(name)                     \
	{                                  \
		printf("Running " #name "\n"); \
		name##_test();                 \
	}

int main()
{
	printf("Running tests\n");

	Test(Arena);
	Test(Buffer);
	Test(String);
	Test(StaticStack);
	Test(CircularBuffer);
	Test(Bitset);
	Test(StaticVector);
	Test(StaticMap);
	Test(ObjectPool);
	Test(RingBuffer);

	return 0;
}

void Printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	vfprintf(stdout, format, args);

	va_end(args);
}

void Bugcheck(const char *file, const char *line, const char *format, ...)
{
	printf("Kernel Bugcheck\n");
	printf("\n%s\n%s\n", file, line);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	printf("\n");
	va_end(args);

	fflush(stdout);
	__debugbreak();
}
