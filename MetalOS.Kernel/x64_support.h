#pragma once

extern "C"
{
	void __stdcall x64_ltr(uint16_t entry);

	//Segment Registers RW
	UINT16 __stdcall x64_ReadCS();
	void __stdcall x64_WriteCS(UINT16 segment_selector);
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
}

