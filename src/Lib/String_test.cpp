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
