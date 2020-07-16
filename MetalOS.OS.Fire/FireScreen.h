#pragma once
#include <MetalOS.h>
#include "Matrix.h"

//DOOM Fire implmentation
//http://fabiensanglard.net/doom_fire_psx/
class FireScreen
{
public:
	FireScreen(size_t height, size_t width);

	void Initialize();
	void Update();
	void Draw();

	const void* GetBuffer() const
	{
		return m_buffer.Buffer();
	}

private:
	static const uint32_t PixelSize = 4;
	static const Color FireColors[];
	static const size_t FireColorsCount = 37;

	void SpreadFire(Point2D point);

	Matrix<uint8_t> m_indexes;
	Matrix<Color> m_buffer;
};

