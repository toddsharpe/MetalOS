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
	static const Color FireColors[];
	static const size_t FireColorsCount = 37;
	static uint32_t ThreadLoop(void* arg);

	void Draw();
	void Update();

	//DOOM Fire implmentation
	//http://fabiensanglard.net/doom_fire_psx/
	//JS implementation uses a index 'firePixels' which is as big as the whole screen
	//To save room, stuff the index into the reserved field in the color
	void DoFire();
	void SpreadFire(Point2D point);

	Matrix<uint8_t> m_indexes;
	Display& m_display;
};

