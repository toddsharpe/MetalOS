#include "PageTablesPool.h"
#include <stdio.h>
#include <crt_string.h>

int main(int argc, char** argv)
{
	////Allocate pool of twice the size, our pool will start half way through
	//LPVOID baseAddr = VirtualAlloc(NULL, ReservedPageTableSpace * 2, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	//RtlZeroMemory(baseAddr, ReservedPageTableSpace * 2);
	//
	//printf("Virtual Address: 0x%016x Size:0x%08x\n", baseAddr, ReservedPageTableSpace * 2);
	//printf("Physical Address: 0 -> 0x%016x -> 0x%016x\n", ReservedPageTableSpace, ReservedPageTableSpace * 2);

	//PageTablesPool pool((UINT64)baseAddr + ReservedPageTableSpace, ReservedPageTableSpace, ReservedPageTablePages);
	//
	////Get our main page
	//UINT64 address = 0;
	//if (!pool.AllocatePage(&address))
	//{
	//	return -1;
	//}
	//printf("Page 0x%016x\n", address);

	////Test page tables
	//PageTables tables(address);
	//tables.SetPool(&pool);
	//tables.SetVirtualOffset((UINT64)baseAddr);

	////Map 1mb at the 1mb offset
	//tables.MapKernelPages(KernelStart + 0x100000, 0x100000, 10);

	//pool.DeallocatePage(address);

	//UINT64 resolved = tables.ResolveAddress(KernelStart + 0x100000);
	//printf("Resolved: 0x%016x\n", resolved);

	uint8_t buffer[10] = { 0 };
	uint8_t buffer2[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	memcpy(buffer, buffer2, 5);
}

