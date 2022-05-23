#pragma once
#include "Display.h"
#include "Matrix.h"
#include <shared/MetalOS.Types.h>

class LoadingScreen
{
public:
	LoadingScreen(const Display& display);
	void Initialize();

private:
	static const uint32_t PixelSize = 4;
	static const Color FireColors[];
	static const size_t FireColorsCount = 37;
	static size_t ThreadLoop(void* arg);

	void Draw();
	void Update();

	//DOOM Fire implmentation
	//http://fabiensanglard.net/doom_fire_psx/

	void DoFire();
	void SpreadFire(Point2D point);

	const Display& m_display;
	Matrix<uint8_t> m_indexes;
	Matrix<Color> m_buffer;
};

