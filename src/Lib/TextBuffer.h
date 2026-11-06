#pragma once

#include <cstdint>
#include <cstddef>
#include "Lib/String.h"
#include "Assert.h"

//Scrollback of text lines: a fixed ring that wraps long lines and drops the oldest when full, so writes never fail
template <size_t MaxLines, size_t MaxColumns>
class TextBuffer
{
public:
	static_assert(MaxLines > 0 && MaxColumns > 0, "TextBuffer needs at least one line and one column");

	//All zeros, so a global TextBuffer sits in .bss instead of taking its whole size in the image
	constexpr TextBuffer() :
		m_lines(),
		m_attributes(),
		m_lengths(),
		m_first(),
		m_last(),
		m_columns(),
		m_attribute()
	{

	}

	//Wrap width for text written from now on, e.g. a view narrower than MaxColumns
	void SetColumns(const size_t columns)
	{
		m_columns = (columns == 0 || columns > MaxColumns) ? 0 : columns;
	}

	size_t Columns() const
	{
		return m_columns == 0 ? MaxColumns : m_columns;
	}

	//Tag for the characters written from now on (a color, say), kept per character and read back with Attributes
	void SetAttribute(const uint8_t attribute)
	{
		m_attribute = attribute;
	}

	void Write(const cstr_t text)
	{
		for (const char c : text)
			Write(c);
	}

	void Write(const char c)
	{
		switch (c)
		{
			case '\n':
				NewLine();
				break;

			case '\t':
				do
				{
					Put(' ');
				} while (CurrentLength() % TabWidth != 0);
				break;

			default:
				//Other control characters (and bytes outside printable ASCII) would draw as garbage
				if (c >= ' ' && c <= '~')
					Put(c);
				break;
		}
	}

	//Removes the last character of the current line; false if the line is empty
	bool Backspace()
	{
		size_t& length = m_lengths[Slot(m_last)];
		if (length == 0)
			return false;

		length--;
		return true;
	}

	void Clear()
	{
		m_first = 0;
		m_last = 0;
		m_lengths[0] = 0;
	}

	//Lines kept, including the current (last) one, so never zero
	size_t LineCount() const
	{
		return m_last + 1;
	}

	//Index 0 is the oldest line kept
	cstr_t Line(const size_t index) const
	{
		Assert(index <= m_last);
		const size_t slot = Slot(index);
		return { m_lines[slot], m_lengths[slot] };
	}

	size_t CurrentLength() const
	{
		return m_lengths[Slot(m_last)];
	}

	//One attribute per character of Line(index)
	const uint8_t* Attributes(const size_t index) const
	{
		Assert(index <= m_last);
		return m_attributes[Slot(index)];
	}

private:
	static constexpr size_t TabWidth = 4;

	size_t Slot(const size_t index) const
	{
		return (m_first + index) % MaxLines;
	}

	void Put(const char c)
	{
		if (CurrentLength() >= Columns())
			NewLine();

		const size_t slot = Slot(m_last);
		m_attributes[slot][m_lengths[slot]] = m_attribute;
		m_lines[slot][m_lengths[slot]++] = c;
	}

	void NewLine()
	{
		if (m_last == MaxLines - 1)
			m_first = (m_first + 1) % MaxLines;
		else
			m_last++;

		m_lengths[Slot(m_last)] = 0;
	}

	char m_lines[MaxLines][MaxColumns];
	uint8_t m_attributes[MaxLines][MaxColumns];
	size_t m_lengths[MaxLines];
	size_t m_first; //Slot of the oldest line
	size_t m_last; //Index of the current (newest) line, so the line count is m_last + 1
	size_t m_columns; //Wrap width, or 0 for MaxColumns
	uint8_t m_attribute;
};
