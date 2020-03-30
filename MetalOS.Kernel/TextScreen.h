#pragma once

#include "Display.h"
#include "Font.h"
#include "StringPrinter.h"

class TextScreen: public StringPrinter
{
public:
	TextScreen(Display& display);

	void Write(const char* string);

private:
	void WriteCharacter(char c);
	void AdvanceX();
	void ResetX();
	void AdvanceY();
	void ResetY();

	const uint8_t m_fontScale;
	static const Color m_white;
	Point2D m_position;
	Display& m_display;
	Font m_font;
};

