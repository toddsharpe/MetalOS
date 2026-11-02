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

	constexpr TextBuffer() :
		m_lines(),
		m_lengths(),
		m_first(),
		m_count(1),
		m_columns(MaxColumns)
	{

	}

	//Wrap width for text written from now on, e.g. a view narrower than MaxColumns
	void SetColumns(const size_t columns)
	{
		m_columns = (columns == 0 || columns > MaxColumns) ? MaxColumns : columns;
	}

	size_t Columns() const
	{
		return m_columns;
	}

	void Write(const CString& text)
	{
		for (size_t i = 0; i < text.Length; i++)
			Write(text[i]);
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
		size_t& length = m_lengths[Slot(m_count - 1)];
		if (length == 0)
			return false;

		length--;
		return true;
	}

	void Clear()
	{
		m_first = 0;
		m_count = 1;
		m_lengths[0] = 0;
	}

	//Lines kept, including the current (last) one, so never zero
	size_t LineCount() const
	{
		return m_count;
	}

	//Index 0 is the oldest line kept
	CString Line(const size_t index) const
	{
		Assert(index < m_count);
		const size_t slot = Slot(index);
		return CString(m_lines[slot], m_lengths[slot]);
	}

	size_t CurrentLength() const
	{
		return m_lengths[Slot(m_count - 1)];
	}

private:
	static constexpr size_t TabWidth = 4;

	size_t Slot(const size_t index) const
	{
		return (m_first + index) % MaxLines;
	}

	void Put(const char c)
	{
		if (CurrentLength() >= m_columns)
			NewLine();

		const size_t slot = Slot(m_count - 1);
		m_lines[slot][m_lengths[slot]++] = c;
	}

	void NewLine()
	{
		if (m_count == MaxLines)
			m_first = (m_first + 1) % MaxLines;
		else
			m_count++;

		m_lengths[Slot(m_count - 1)] = 0;
	}

	char m_lines[MaxLines][MaxColumns];
	size_t m_lengths[MaxLines];
	size_t m_first; //Slot of the oldest line
	size_t m_count;
	size_t m_columns;
};
