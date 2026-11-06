#include "Lib/String.h"

void String_test()
{
	//A literal's text leaves out the terminator
	static constexpr cstr_t RuntimeDLL = Str("MetalOS-RT.dll");
	static_assert(RuntimeDLL.Size == 14);

	//A buffer's text stops at its terminator, not at the end of the array
	char buffer[128] = {};
	buffer[0] = 'a';
	buffer[1] = 'b';
	Assert(Str(buffer).Size == 2);
	Assert(Str(buffer) == Str("ab"));

	//An array with no terminator is read to its end
	static constexpr char name[4] = { '_', 'S', 'B', '_' };
	Assert(Str(name).Size == 4);
	Assert(Str(name) == Str("_SB_"));

	//C strings, whole and bounded
	static constexpr const char* text = "123a";
	Assert(CStr(text) == Str("123a"));
	Assert(CStr(text, 2) == Str("12"));
	Assert(CStr(nullptr).Size == 0);

	//Comparisons
	Assert(Str("abc") != Str("abd"));
	Assert(Str("abc") != Str("ab"));
	Assert(Str("") == cstr_t());

	//StrCopy: fits exactly (N - 1 chars and the terminator)
	char fits[8];
	Assert(StrCopy(fits, Str("1234567")) == 7);
	Assert(Str(fits) == Str("1234567"));
	Assert(fits[7] == '\0');

	//StrCopy: one char too long is clipped
	char over[8];
	Assert(StrCopy(over, Str("12345678")) == 7);
	Assert(Str(over) == Str("1234567"));

	//StrCopy: replaces what was there, with nothing left behind
	StrCopy(over, Str("hi"));
	Assert(Str(over) == Str("hi"));

	//StrCopy: a long input keeps the first N - 1 chars
	char longInput[300] = {};
	memset(longInput, 'x', sizeof(longInput) - 1);
	char path[128];
	Assert(StrCopy(path, Str(longInput)) == 127);
	Assert(strlen(path) == 127);

	//StrAppend: fills the space left and clips the rest
	char tail[8] = {};
	Assert(StrAppend(tail, Str("abc")) == 3);
	Assert(StrAppend(tail, Str("defghij")) == 7);
	Assert(Str(tail) == Str("abcdefg"));

	//StrAppend: a full string stays as it is
	Assert(StrAppend(tail, Str("z")) == 7);
	Assert(Str(tail) == Str("abcdefg"));

	//StrAppend: an empty append changes nothing
	char rest[8] = {};
	StrAppend(rest, Str("ab"));
	StrAppend(rest, Str(""));
	Assert(Str(rest) == Str("ab"));
}
