#include "Kernel.h"
#include "Assert.h"

#include "TextScreen.h"

#define PositionToPixelScale 10

TextScreen::TextScreen(const Display& display, const Color color) :
	m_position(),
	m_font(),
	m_display(display),
	m_color(color)
{
	
}

void TextScreen::Write(const char* text)
{
	Assert(text != nullptr);

	while (*text != '\0')
	{
		if (*text == '\n')
		{
			m_position.Y = (m_position.Y += PixelScale);
			m_position.X = 0;
		}
		else if (*text != '\r')
		{
			WriteCharacter(*text);
			m_position.X = (m_position.X += PixelScale);
		}

		text++;
	}
}

//TODO: edgecases
void TextScreen::WriteCharacter(const char c)
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
				Rectangle rect;
				rect.X = (m_position.X + (8 - x - 1)) * FontScale;
				rect.Y = (m_position.Y + y) * FontScale;
				rect.Width = FontScale;
				rect.Height = FontScale;
				m_display.ColorRectangle(Colors::White, rect);
			}

			mask = mask >> 1;
			x++;
		}
	}
}

