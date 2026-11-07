#include "user/MetalOS.h"
#include <stdio.h>

int main(int agrc, char** argv)
{
	UNUSED(agrc);
	UNUSED(argv);
	SystemTime time = {};
	AssertSuccess(GetSystemTime(&time));

	printf("The current time is:\n");
	printf("%d:%d:%d.%d\n", time.Hour, time.Minute, time.Second, time.Milliseconds);

	return 0;
}