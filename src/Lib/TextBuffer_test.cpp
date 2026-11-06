#include "Lib/TextBuffer.h"

void TextBuffer_test()
{
	//Starts with one empty line
	{
		TextBuffer<4, 8> text;
		AssertEqual(text.LineCount(), (size_t)1);
		Assert(text.Line(0) == "");
	}

	//Newlines start lines; carriage returns and other control characters are dropped
	{
		TextBuffer<4, 8> text;
		text.Write("ab\r\ncd\x01\n");
		AssertEqual(text.LineCount(), (size_t)3);
		Assert(text.Line(0) == "ab");
		Assert(text.Line(1) == "cd");
		Assert(text.Line(2) == "");
	}

	//Long lines wrap at the column count, without an extra blank line when a newline follows a full line
	{
		TextBuffer<4, 8> text;
		text.SetColumns(4);
		text.Write("abcdefgh\nij");
		AssertEqual(text.LineCount(), (size_t)3);
		Assert(text.Line(0) == "abcd");
		Assert(text.Line(1) == "efgh");
		Assert(text.Line(2) == "ij");
	}

	//Columns are clamped to the storage width
	{
		TextBuffer<4, 8> text;
		text.SetColumns(100);
		AssertEqual(text.Columns(), (size_t)8);
		text.SetColumns(0);
		AssertEqual(text.Columns(), (size_t)8);
	}

	//Tabs expand to the next 4-column stop
	{
		TextBuffer<4, 16> text;
		text.Write("a\tb");
		Assert(text.Line(0) == "a   b");
	}

	//When full, the oldest lines are dropped and writes keep going
	{
		TextBuffer<3, 8> text;
		text.Write("1\n2\n3\n4\n5");
		AssertEqual(text.LineCount(), (size_t)3);
		Assert(text.Line(0) == "3");
		Assert(text.Line(1) == "4");
		Assert(text.Line(2) == "5");
	}

	//Far more text than fits, as from a chatty child process
	{
		TextBuffer<4, 8> text;
		for (size_t i = 0; i < 1000; i++)
			text.Write("0123456789\n");
		AssertEqual(text.LineCount(), (size_t)4);
		Assert(text.Line(2) == "89");
		Assert(text.Line(3) == "");
	}

	//Backspace edits only the current line
	{
		TextBuffer<4, 8> text;
		text.Write("ab\nc");
		Assert(text.Backspace());
		Assert(text.Line(1) == "");
		Assert(!text.Backspace());
		Assert(text.Line(0) == "ab");
	}

	//Clear leaves one empty line
	{
		TextBuffer<4, 8> text;
		text.Write("ab\ncd");
		text.Clear();
		AssertEqual(text.LineCount(), (size_t)1);
		Assert(text.Line(0) == "");
	}

	//Each character keeps the attribute it was written with, also across a wrap
	{
		TextBuffer<4, 4> text;
		text.Write("ab");
		text.SetAttribute(3);
		text.Write("cde");
		AssertEqual(text.Attributes(0)[1], (uint8_t)0);
		AssertEqual(text.Attributes(0)[2], (uint8_t)3);
		AssertEqual(text.Attributes(1)[0], (uint8_t)3);
	}
}
