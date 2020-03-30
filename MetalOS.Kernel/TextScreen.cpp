#include "TextScreen.h"

#include <crt_stdio.h>
#include "Main.h"

#define PositionToPixelScale 10

const Color TextScreen::m_white = { 0xFF, 0xFF, 0xFF, 0x00 };

TextScreen::TextScreen(Display& display) : m_display(display), m_fontScale(1)
{
	m_position = { 0 };
}

void TextScreen::Write(const char* text)
{
	Assert(text != nullptr);

	while (*text != '\0')
	{
		if (*text == '\n')
		{
			AdvanceY();
			ResetX();
		}
		else if (*text != '\r')
		{
			WriteCharacter(*text);
			AdvanceX();
		}

		text++;
	}
}

//TODO: edgecases
void TextScreen::WriteCharacter(char c)
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

void TextScreen::ResetX()
{
	m_position.X = 0;
}

void TextScreen::AdvanceY()
{
	m_position.Y = (m_position.Y += PositionToPixelScale);
}

void TextScreen::ResetY()
{
	m_position.Y = 0;
}

void TextScreen::AdvanceX()
{
	m_position.X = (m_position.X += PositionToPixelScale);
}


