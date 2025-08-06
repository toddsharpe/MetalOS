#include "Lib/String.h"

void String_test()
{
	//Construct from char array
	static constexpr CString RuntimeDLL = "mosrt.dll";
	
	//Assign from char array
	static constexpr char s[] = "abc";
	static constexpr CString s1(s);
	Assert(s1 == "abc");

	//Assign from char pointer/length
	static constexpr char* t = "def";
	static constexpr CString t1(t, 3);
	Assert(t1 == "def");

	//Assign from char pointer
	static constexpr char* n = "123";
	static constexpr CString n1(n);
	Assert(n1 == "123");

	//Assign from char pointer
	char* m = "123a";
	StaticString<128> copy = {};
	copy.AppendClip(m);
	Assert(copy == "123a");

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
}
