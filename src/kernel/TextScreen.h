#pragma once

#include "Display.h"
#include "Font.h"
#include "StringPrinter.h"

class TextScreen: public StringPrinter
{
public:
	TextScreen(const Display& display, const Color color = Colors::White);

	void Write(const char* string);

private:
	static constexpr size_t PixelScale = 10;
	static constexpr size_t FontScale = 1;

	void WriteCharacter(const char c);

	Point2D m_position;
	const Font m_font;
	const Color m_color;
	const Display& m_display;
};

