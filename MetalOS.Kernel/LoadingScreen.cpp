#include "LoadingScreen.h"
#include "Main.h"
#include <crt_stdio.h>

#define PositionToPixelScale 10

const Color LoadingScreen::m_white = { 0xFF, 0xFF, 0xFF, 0x00 };

LoadingScreen::LoadingScreen(Display& display) : m_display(display), m_fontScale(1)
{
	m_position = { 0 };
}

void LoadingScreen::WriteLineFormat(const char* format, ...)
{
	char buffer[255];

	va_list ap;

	va_start(ap, format);
	int retval = crt_vsprintf(buffer, format, ap);
	buffer[retval] = '\0';
	va_end(ap);

	WriteLine(buffer);
}

void LoadingScreen::WriteLine(const char* s)
{
	LoadingScreen::WriteText(s);
	AdvanceY();
	ResetX();
}

void LoadingScreen::WriteText(const char* text)
{
	Assert(text != nullptr);

	while (*text != '\0')
	{
		WriteCharacter(*text);
		AdvanceX();

		text++;
	}
}

//TODO: edgecases
void LoadingScreen::WriteCharacter(char c)
{
	const char* map = m_font.GetCharacterMap(c);
	uint8_t size = 8; // TODO: get from font

	for (size_t y = 0; y < size; y++)
	{
		//Line is reversed
		char line = map[y];

		size_t x = 0;
		unsigned char mask = 0x80;
		while (mask > 0)
		{
			if ((line & mask) != 0)
			{
				Point2D origin;
				origin.X = (m_position.X + (8 - x - 1)) * m_fontScale;
				origin.Y = (m_position.Y + y) * m_fontScale;
				
				Rectangle rect;
				rect.P1 = origin;
				rect.P2.X = origin.X + m_fontScale;
				rect.P2.Y = origin.Y + m_fontScale;
				m_display.ColorRectangle(m_white, &rect);
			}

			mask = mask >> 1;
			x++;
		}
	}
}

void LoadingScreen::ResetX()
{
	m_position.X = 0;
}

void LoadingScreen::AdvanceY()
{
	m_position.Y = (m_position.Y += PositionToPixelScale);
}

void LoadingScreen::ResetY()
{
	m_position.Y = 0;
}

void LoadingScreen::AdvanceX()
{
	m_position.X = (m_position.X += PositionToPixelScale);
}


