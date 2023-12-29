#include "msvc.h"
#include "FireScreen.h"

#include "Assert.h"
#include <stdlib.h>

using namespace Graphics;

FireScreen::FireScreen(const size_t height, const size_t width) :
	m_height(height / PixelSize),
	m_width(width / PixelSize),
	m_indexes()
{
	Assert(height % PixelSize == 0);
	Assert(width % PixelSize == 0);
}

void FireScreen::Initialize()
{
	//Allocate index array
	m_indexes = new size_t[m_height * m_width];

	//Set bottom of indexes to highest index
	const size_t bottomY = m_height - 1;
	for (size_t x = 0; x < m_width; x++)
		m_indexes[bottomY * m_width + x] = FireColorsCount - 1;
}

void FireScreen::Update()
{
	for (size_t x = 0; x < m_width; x++)
		for (size_t y = 1; y < m_height; y++)
			SpreadFire({ x, y });
}

void FireScreen::SpreadFire(Point2D point)
{
	size_t index = m_indexes[point.Y * m_width + point.X];
	if (index == 0)
	{
		m_indexes[(point.Y - 1) * m_width + point.X] = 0;
	}
	else
	{
		const int randIndex = rand() & 3;

		size_t dstX = point.X;
		if (((randIndex & 3) == 0) && (dstX > 0))
		{
			dstX--;
		}
		else if (((randIndex & 3) == 3) && (dstX < m_width - 1))
		{
			dstX++;
		}

		const Point2D dstPoint = { dstX, point.Y - 1 };
		m_indexes[dstPoint.Y * m_width + dstPoint.X] = index - (randIndex & 1);
	}
}

void FireScreen::Draw(FrameBuffer& frameBuffer)
{
	//Populate graphics buffer based on indexes
	for (size_t x = 0; x < m_width; x++)
		for (size_t y = 0; y < m_height; y++)
		{
			const size_t index = m_indexes[y * m_width + x];
			const Color color = FireColors[index];

			frameBuffer.DrawRectangle(color, { x * PixelSize, y * PixelSize, PixelSize, PixelSize });
		}
}
