#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include "Assert.h"

#define Test(name)                     \
	{                                  \
		void name##_test();            \
		printf("Running " #name "\n"); \
		name##_test();                 \
	}

int main()
{
	printf("Running tests\n");

	Test(AddressSpace);
	Test(Arena);
	Test(BlockAllocator);
	Test(Span);
	Test(String);
	Test(Serializer);
	Test(CircularBuffer);
	Test(Bitset);
	Test(StaticVector);
	Test(StaticMap);
	Test(RingBuffer);
	Test(SharedRing);
	Test(StaticHeap);
	Test(IntrusiveList);
	Test(TextBuffer);
	Test(Path);
	Test(WinPE);
	Test(Pdb);
	Test(ArpCache);
	Test(Checksum);
	Test(StackWalk);
	Test(PebModules);
	Test(IPv4);
	Test(Socket);
	Test(Router);

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
