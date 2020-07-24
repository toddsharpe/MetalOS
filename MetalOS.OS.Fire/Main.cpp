#include <MetalOS.h>
#include <stdlib.h>
#include "Matrix.h"
#include "FireScreen.h"

int main(int argc, char** argv)
{
	Handle h = CreateWindow("Fire");

	Rectangle rect = { 0 };
	GetWindowRect(h, rect);
	DebugPrintf("Rect: (0x%x,0x%x) x (0x%x,0x%x)\n",
		rect.P1.X, rect.P1.Y, rect.P2.X, rect.P2.Y);

	const size_t width = rect.P2.X - rect.P1.X;
	const size_t height = rect.P2.Y - rect.P1.Y;
	
	FireScreen screen(height, width);
	screen.Initialize();
	screen.Draw();
	SetScreenBuffer(screen.GetBuffer());

	while (true)
	{
		screen.Update();
		screen.Draw();
		SetScreenBuffer(screen.GetBuffer());
	}
}


