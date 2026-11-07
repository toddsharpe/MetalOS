#include "user/MetalOS.h"
#include <stdio.h>

static void DivZero()
{
	//Div zero, by a volatile so the division can't be optimized away
	volatile int divisor = 0;
	printf("%d\n", 1 / divisor);
}

static void NullDereference()
{
	void* ptr = nullptr;
	*(int*)ptr = 1;
}

void DebugBreak()
{
	__debugbreak();
}

//Faults inside MetalOS-CRT.dll (printf reading the string), so the trace goes through a DLL's frames
static void DllFault()
{
	printf("%s\n", (const char*)1);
}

int main(int argc, char** argv)
{
	printf("Userspace crash demo.");
	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++)
		printf("argv[%d]: %s\n", i, argv[i]);
	printf("\n");
	
	if (argc < 2)
	{
		printf("Usage: crash [null, div0, brk, dll]");
		return -1;
	}

	if (strcmp(argv[1], "div0") == 0)
	{
		DivZero();
	}
	else if (strcmp(argv[1], "null") == 0)
	{
		NullDereference();
	}
	else if (strcmp(argv[1], "brk") == 0)
	{
		DebugBreak();
	}
	else if (strcmp(argv[1], "dll") == 0)
	{
		DllFault();
	}
	else
	{
		printf("Unknown type: %s\n", argv[1]);
	}

	return 0;
}
