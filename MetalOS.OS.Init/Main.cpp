#include "MetalOS.h"

int main(int argc, char** argv)
{
	SystemInfo sysInfo = { 0 };
	GetSystemInfo(&sysInfo);
	
	ProcessInfo info = { 0 };
	GetProcessInfo(&info);

	int i = 0;
	while (1)
	{
		i++;
	}
}
