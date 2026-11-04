#include "Lib/String.h"

void String_test()
{
	//Construct from char array
	static constexpr CString RuntimeDLL = "MetalOS-RT.dll";
	
	//Assign from char array
	static constexpr char s[] = "abc";
	static constexpr CString s1(s);
	Assert(s1 == "abc");

	//Assign from char pointer/length
	static constexpr const char* t = "def";
	static constexpr CString t1(t, 3);
	Assert(t1 == "def");

	//Assign from char pointer
	static constexpr const char* n = "123";
	static constexpr CString n1(n);
	Assert(n1 == "123");

	//Assign from char pointer
	const char* m = "123a";
	StaticString<128> copy = {};
	copy.AppendClip(m);
	Assert(copy == "123a");

	//AppendClip: exactly fits (Capacity - 1 chars plus the null terminator)
	StaticString<8> fits;
	fits.AppendClip("1234567");
	Assert(fits == "1234567");
	Assert(fits.c_str()[7] == '\0');

	//AppendClip: one char too long is clipped, not asserted
	StaticString<8> over;
	over.AppendClip("12345678");
	Assert(over == "1234567");
	Assert(over.c_str()[7] == '\0');

	//AppendClip: a long command line or path keeps the first Capacity - 1 chars
	char longInput[300] = {};
	memset(longInput, 'x', sizeof(longInput) - 1);
	StaticString<128> path;
	path.AppendClip(longInput);
	Assert(path.Count() == 127);
	Assert(strlen(path.c_str()) == 127);

	//AppendClip: appending to a non-empty string clips to the space left
	StaticString<8> tail;
	tail.Append("abc");
	tail.AppendClip("defghij");
	Assert(tail == "abcdefg");

	//AppendClip: a full string stays as it is
	tail.AppendClip("z");
	Assert(tail == "abcdefg");

	//AppendClip: maxLength limits the copy when it is shorter than the space left
	StaticString<8> limited;
	limited.AppendClip("abcdef", 3);
	Assert(limited == "abc");

	//AppendClip: the space left limits the copy when maxLength is longer
	limited.AppendClip("defghij", 10);
	Assert(limited == "abcdefg");
	Assert(limited.Count() == 7);

	//Appending
	StaticString<64> a;
	a.Append('A');
	Assert(a == "A");
	a.Append('B');
	Assert(a == "AB");
	a.Append("CD");
	Assert(a == "ABCD");

	//Assigning to static string
	StaticString<32> newS(n1);

	//Assignment
	//TODO(tsharpe): Why doesnt assignment work here?
	StaticString<32> b;
	b.Set("test");
	Assert(b == "test");

	const bool c = b != "test";
	Assert(!c);

	b.Pop();
	Assert(b == "tes");

	// Set replaces existing content (not appends)
	StaticString<32> d;
	d.Set("initial");
	Assert(d == "initial");
	Assert(d.Count() == 7);
	d.Set("replaced");
	Assert(d == "replaced");
	Assert(d.Count() == 8);

	// Set with shorter string — old content must not linger
	d.Set("hi");
	Assert(d == "hi");
	Assert(d.Count() == 2);

	// Set from CString
	static constexpr CString src("hello");
	StaticString<32> e;
	e.Set(src);
	Assert(e == "hello");
	Assert(e.Count() == 5);

	// Set after Set — count stays correct
	e.Set("world!");
	Assert(e == "world!");
	Assert(e.Count() == 6);
}
