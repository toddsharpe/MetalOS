#pragma once

//extern const uint64_t x64_CONTEXT_SIZE;
extern "C"
{
	//Context
	extern uint64_t x64_CONTEXT_SIZE;
	int x64_save_context(void* context);//Returns 0
	__declspec(noreturn) int x64_load_context(void* context);//Returns 1
	void x64_init_context(void* context, void* stack, void* entry);

	__declspec(noreturn) int x64_start_user_thread(void* context, void* teb);
	
	//Omitted from intrinsics
	void __stdcall x64_ltr(uint16_t entry);
	void __stdcall x64_sti(); //_enable?
	void __stdcall x64_cli(); //_disable?
	void __stdcall x64_swapgs();

	//Segment Registers RW
	uint16_t __stdcall x64_ReadCS();
	uint16_t __stdcall x64_ReadGS();
	uint16_t __stdcall x64_ReadSS();

	void x64_pause();

	//Helpers
	cpu_flags_t x64_disable_interrupts();
	void x64_restore_flags(cpu_flags_t flags);
	void x64_update_segments(uint16_t data_selector, uint16_t code_selector);
}

