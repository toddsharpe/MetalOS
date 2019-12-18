#pragma once

#define def_x64_GPR_RW(name) \
	UINT64 __stdcall x64_Read ## name ## (); \
	void __stdcall x64_Write ## name ## (UINT64 value);

extern "C"
{
	//Omitted from intrinsics
	void __stdcall x64_ltr(uint16_t entry);
	UINT64 __stdcall x64_rflags();
	void __stdcall x64_sti();
	void __stdcall x64_cli();

	//Segment Registers RW
	UINT16 __stdcall x64_ReadCS();
	UINT16 __stdcall x64_ReadDS();
	void __stdcall x64_WriteDS(UINT16 segment_selector);
	UINT16 __stdcall x64_ReadES();
	void __stdcall x64_WriteES(UINT16 segment_selector);
	UINT16 __stdcall x64_ReadFS();
	void __stdcall x64_WriteFS(UINT16 segment_selector);
	UINT16 __stdcall x64_ReadGS();
	void __stdcall x64_WriteGS(UINT16 segment_selector);
	UINT16 __stdcall x64_ReadSS();
	void __stdcall x64_WriteSS(UINT16 segment_selector);

	//General Purpose Register (RW)
	def_x64_GPR_RW(Rax);
	def_x64_GPR_RW(Rcx);
	def_x64_GPR_RW(Rdx);
	def_x64_GPR_RW(Rbx);
	def_x64_GPR_RW(Rsi);
	def_x64_GPR_RW(Rdi);
	def_x64_GPR_RW(Rsp);
	def_x64_GPR_RW(Rbp);
	def_x64_GPR_RW(R8);
	def_x64_GPR_RW(R9);
	def_x64_GPR_RW(R10);
	def_x64_GPR_RW(R11);
	def_x64_GPR_RW(R12);
	def_x64_GPR_RW(R13);
	def_x64_GPR_RW(R14);
	def_x64_GPR_RW(R15);

	//Helpers
	void __stdcall x64_update_segments(UINT16 data_selector, UINT16 code_selector);
}

