#pragma once

#include <cstdint>

typedef size_t cpu_status_t;
typedef uintptr_t paddr_t;

extern "C"
{
	void ArchMain();
	void ArchInitialize();

	size_t ArchContextSize();
	size_t ArchStackReserve();
	void ArchInitContext(void* context, void* const entry, void* const stack);
	bool ArchSaveContext(void* context); //Returns 0
	__declspec(noreturn) bool ArchLoadContext(void* context); //Returns 1
	__declspec(noreturn) void ArchUserThreadStart(void* context, void* teb);

	void ArchSetPagingRoot(paddr_t root);
	void ArchSetInterruptStack(void* stack);
	void ArchSetUserCpuContext(void* context);

	void ArchPause();
	void ArchWait();
	uint32_t ArchReadPort(uint32_t port, uint8_t width);
	void ArchWritePort(uint32_t port, uint32_t value, uint8_t width);
	void ArchEnableInterrupts();
	cpu_status_t ArchDisableInterrupts();
	void ArchRestoreFlags(cpu_status_t val);
}

