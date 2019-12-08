#pragma once

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

	//Register RW
	UINT64 __stdcall x64_ReadSP();
	UINT64 __stdcall x64_ReadRCX();
	UINT64 __stdcall x64_ReadRDX();

	//Helpers
	void __stdcall x64_update_segments(UINT16 data_selector, UINT16 code_selector);
}

