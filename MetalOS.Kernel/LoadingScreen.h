#pragma once
#include "Display.h"
#include "Matrix.h"
#include "MetalOS.h"

class LoadingScreen
{
public:
	LoadingScreen(Display& display);
	void Initialize();

private:
	static const uint32_t PixelSize = 4;
	static const Color FireColors[];
	static const size_t FireColorsCount = 37;
	static uint32_t ThreadLoop(void* arg);

	void Draw();
	void Update();

	//DOOM Fire implmentation
	//http://fabiensanglard.net/doom_fire_psx/

	void DoFire();
	void SpreadFire(Point2D point);

	Display& m_display;
	Matrix<uint8_t> m_indexes;
	Matrix<Color> m_buffer;
};

