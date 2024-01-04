#include <MetalOS.h>
#include <stdio.h>
#include <string.h>

void DivZero()
{
	//Div zero
	int x = 1;
	int y = 0;
	int z = x / y;
}

void NullDereference()
{
	void* ptr = nullptr;
	*(int*)ptr = 1;
}

void DebugBreak()
{
	__debugbreak();
}

int main(int argc, char** argv)
{
	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++)
		printf("argv[%d]: %s\n", i, argv[i]);
	
	if (argc < 2)
	{
		printf("Usage: crash [null, div0]");
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
	else
	{
		printf("Unknown type: %s\n", argv[1]);
	}
}
