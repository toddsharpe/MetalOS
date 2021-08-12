#include "FireScreen.h"
#include <stdlib.h>

//BGR pixels
const Color FireScreen::FireColors[] =
{
	{ 0x07, 0x07, 0x07, 0x00},
	{ 0x07, 0x07, 0x1F, 0x00},
	{ 0x07, 0x0F, 0x2F, 0x00},
	{ 0x07, 0x0F, 0x47, 0x00},
	{ 0x07, 0x17, 0x57, 0x00},
	{ 0x07, 0x1F, 0x67, 0x00},
	{ 0x07, 0x1F, 0x77, 0x00},
	{ 0x07, 0x27, 0x8F, 0x00},
	{ 0x07, 0x2F, 0x9F, 0x00},
	{ 0x07, 0x3F, 0xAF, 0x00},
	{ 0x07, 0x47, 0xBF, 0x00},
	{ 0x07, 0x47, 0xC7, 0x00},
	{ 0x07, 0x4F, 0xDF, 0x00},
	{ 0x07, 0x57, 0xDF, 0x00},
	{ 0x07, 0x57, 0xDF, 0x00},
	{ 0x07, 0x5F, 0xD7, 0x00},
	{ 0x07, 0x5F, 0xD7, 0x00},
	{ 0x0F, 0x67, 0xD7, 0x00},
	{ 0x0F, 0x6F, 0xCF, 0x00},
	{ 0x0F, 0x77, 0xCF, 0x00},
	{ 0x0F, 0x7F, 0xCF, 0x00},
	{ 0x17, 0x87, 0xCF, 0x00},
	{ 0x17, 0x87, 0xC7, 0x00},
	{ 0x17, 0x8F, 0xC7, 0x00},
	{ 0x1F, 0x97, 0xC7, 0x00},
	{ 0x1F, 0x9F, 0xBF, 0x00},
	{ 0x1F, 0x9F, 0xBF, 0x00},
	{ 0x27, 0xA7, 0xBF, 0x00},
	{ 0x27, 0xA7, 0xBF, 0x00},
	{ 0x2F, 0xAF, 0xBF, 0x00},
	{ 0x2F, 0xAF, 0xB7, 0x00},
	{ 0x2F, 0xB7, 0xB7, 0x00},
	{ 0x37, 0xB7, 0xB7, 0x00},
	{ 0x6F, 0xCF, 0xCF, 0x00},
	{ 0x9F, 0xDF, 0xDF, 0x00},
	{ 0xC7, 0xEF, 0xEF, 0x00},
	{ 0xFF, 0xFF, 0xFF, 0x00},
};

FireScreen::FireScreen(const size_t height, const size_t width) :
	m_indexes(height / PixelSize, width / PixelSize),
	m_buffer(height, width)
{
	Assert(height % PixelSize == 0);
	Assert(width % PixelSize == 0);

	static_assert(sizeof(FireScreen::FireColors) == FireColorsCount * sizeof(Color), "Invalid FireColors array");
}

void FireScreen::Initialize()
{
	//Initialize indexes to 0
	for (size_t y = 0; y < m_indexes.GetHeight(); y++)
		for (size_t x = 0; x < m_indexes.GetWidth(); x++)
			m_indexes.Set({ x, y }, 0);

	//Set bottom of indexes to highest index
	const size_t bottomY = m_indexes.GetHeight() - 1;
	for (size_t x = 0; x < m_indexes.GetWidth(); x++)
		m_indexes.Set({ x, bottomY }, FireColorsCount - 1);
}

void FireScreen::Update()
{
	for (size_t x = 0; x < m_indexes.GetWidth(); x++)
		for (size_t y = 1; y < m_indexes.GetHeight(); y++)
			SpreadFire({ x, y });
}

void FireScreen::SpreadFire(Point2D point)
{
	uint8_t index = m_indexes.Get(point);
	if (index == 0)
	{
		m_indexes.Set({ point.X, point.Y-- }, 0);
	}
	else
	{
		const size_t src = point.Y * m_indexes.GetWidth() + point.X;
		const int randIndex = rand() & 3;

		size_t dstX = point.X;
		if (((randIndex & 3) == 0) && (dstX > 0))
		{
			dstX--;
		}
		else if (((randIndex & 3) == 3) && (dstX < m_indexes.GetWidth() - 1))
		{
			dstX++;
		}

		const Point2D dstPoint = { dstX, point.Y - 1 };
		m_indexes.Set(dstPoint, index - (randIndex & 1));
	}
}

void FireScreen::Draw()
{
	//Populate graphics buffer based on indexes
	for (size_t x = 0; x < m_indexes.GetWidth(); x++)
		for (size_t y = 0; y < m_indexes.GetHeight(); y++)
		{
			const uint8_t index = m_indexes.Get({ x, y });
			const Color c = FireColors[index];

			//Set region scaled up
			for (size_t i = 0; i < PixelSize; i++)
				for (size_t j = 0; j < PixelSize; j++)
				{
					m_buffer.Set({ x * PixelSize + i, y * PixelSize + j }, c);
				}
		}
}
