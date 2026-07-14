#include "user/MetalOS.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	ProcessListEntry procs[64] = {};
	size_t count = 0;
	if (GetProcesses(procs, 64, &count) != SyscallResult::Success)
	{
		printf("ps: failed to enumerate processes\n");
		return 1;
	}

	printf("%-20s %6s %10s %-10s\n", "NAME", "ID", "VIRT(KB)", "STATE");
	for (size_t i = 0; i < count; i++)
	{
		const ProcessListEntry& p = procs[i];
		printf("%-20s %6u %10u %-10s\n", p.Name, p.Id, (uint32_t)(p.VirtualSize / 1024), StateName(p.State));
	}

	return 0;
}
