#include <shared/MetalOS.Types.h>
#include <user/MetalOS.h>
#include <user/Debug.h>
#include <stdio.h>

int main(int agrc, char** argv)
{
	SystemTime time = {};
	AssertSuccess(GetSystemTime(time));

	printf("The current time is:\n");
	printf("%d:%d:%d.%d\n", time.Hour, time.Minute, time.Second, time.Milliseconds);
}