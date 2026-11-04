#include "Lib/Path.h"

void Path_test()
{
	//The directory keeps its trailing slash and is terminated right after it, even in a buffer that isn't zeroed
	{
		wchar_t buffer[32];
		for (wchar_t& c : buffer)
			c = L'X';
		GetDirectoryName(L"\\EFI\\Boot\\BOOTX64.EFI", buffer);
		Assert(wcscmp(buffer, L"\\EFI\\Boot\\") == 0);
		Assert(buffer[11] == L'X');
	}

	//A file at the root leaves just the slash
	{
		wchar_t buffer[8];
		for (wchar_t& c : buffer)
			c = L'X';
		GetDirectoryName(L"\\moskrnl.exe", buffer);
		Assert(wcscmp(buffer, L"\\") == 0);
		Assert(buffer[2] == L'X');
	}
}
