#include "LoadingScreen.h"
#include "Main.h"

#define PositionToPixelScale 10

const Color LoadingScreen::m_white = { 0xFF, 0xFF, 0xFF, 0x00 };

LoadingScreen::LoadingScreen(Display& display) : m_display(display), m_fontScale(4)
{
	m_position = { 0 };
}

void LoadingScreen::WriteText(const char* text)
{
	Assert(text != nullptr);

	while (*text != '\0')
	{
		WriteCharacter(*text);
		AdvancePositionX();

		text++;
	}
}

//TODO: edgecases
void LoadingScreen::WriteCharacter(char c)
{
	const char* map = m_font.GetCharacterMap(c);
	UINT32 size = 8; // TODO: get from font

	Point2D pixelPoint;
	pixelPoint.X = m_position.X * PositionToPixelScale * m_fontScale;
	pixelPoint.Y = m_position.Y * PositionToPixelScale * m_fontScale;

	for (UINT32 i = 0; i < size; i++)
	{
		char line = map[i];

		unsigned char mask = 0x80;
		while (mask > 0)
		{
			if ((line & mask) != 0)
			{
				Rectangle rect;
				rect.P1 = pixelPoint;
				rect.P2.X = pixelPoint.X + (m_fontScale - 1);
				rect.P2.Y = pixelPoint.Y + (m_fontScale - 1);
				m_display.ColorRectangle(m_white, &rect);
			}

			pixelPoint.X += 1 * (m_fontScale - 1);
			mask = mask >> 1;
		}

		//Reset horizontal position
		pixelPoint.X = m_position.X * PositionToPixelScale * m_fontScale;

		pixelPoint.Y += 1 * (m_fontScale - 1);
	}
}

void LoadingScreen::AdvancePositionX()
{
	m_position.X = (m_position.X++) % 80;
}


