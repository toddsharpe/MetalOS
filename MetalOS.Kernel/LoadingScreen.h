#pragma once

#include "Display.h"
#include "Font.h"

class LoadingScreen
{
public:
	LoadingScreen(Display& display);

	void WriteLine(const char* format, ...);
	void WriteLine(const char* format, va_list args);
	void Write(const char* format, ...);
	void Write(const char* format, va_list args);

	void WriteCharacter(char c);

	void AdvanceX();
	void ResetX();
	void AdvanceY();
	void ResetY();

private:
	void WriteText(const char* text);

	const uint8_t m_fontScale;
	static const Color m_white;
	Point2D m_position;
	Display& m_display;
	Font m_font;
};

