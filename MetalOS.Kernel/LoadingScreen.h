#pragma once

#include "Display.h"
#include "Font.h"

class LoadingScreen
{
public:
	LoadingScreen(Display& display);

	void WriteLineFormat(const char* format, ...);
	void WriteLineFormat(const char* format, va_list args);
	void WriteLine(const char* s);
	void WriteText(const char* text);
	void WriteCharacter(char c);

	void AdvanceX();
	void ResetX();
	void AdvanceY();
	void ResetY();

private:
	const uint8_t m_fontScale;
	static const Color m_white;
	Point2D m_position;
	Display& m_display;
	Font m_font;
};

