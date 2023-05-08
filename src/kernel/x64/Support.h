#pragma once

extern "C"
{
	void _pause();
	void _ltr(uint16_t entry);
	void _sti();
	
	cpu_flags_t DisableInterrupts();
	void RestoreFlags(cpu_flags_t flags);

	__declspec(noreturn)
	void UpdateSegments(uint16_t data_selector, uint16_t code_selector);
}
