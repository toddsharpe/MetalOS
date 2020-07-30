#pragma once

#include <cstdint>

typedef size_t cpu_status_t;

extern "C"
{
	void ArchMain();
	void ArchInitialize();

	size_t ArchContextSize();
	void ArchInitContext(void* context, void* entry, void* stack);
	bool ArchSaveContext(void* context); //Returns 0
	__declspec(noreturn) bool ArchLoadContext(void* context); //Returns 1
	__declspec(noreturn) void ArchUserThreadStart(void* context, void* stack, void* teb);

	void ArchSetInterruptStack(void* stack);
	void ArchSetKernelCpuContext(void* context);
	void ArchSetUserCpuContext(void* context);

	void ArchPause();
	uint32_t ArchReadPort(uint32_t port, uint8_t width);
	void ArchWritePort(uint32_t port, uint32_t value, uint8_t width);
	cpu_status_t ArchDisableInterrupts();
	void ArchRestoreFlags(cpu_status_t val);
}

