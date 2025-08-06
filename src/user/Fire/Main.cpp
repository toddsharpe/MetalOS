#include "user/MetalOS.h"
#include "user/GUI.h"
#include "User/Fire/FireScreen.h"

#include "user/Fire/FireScreen.cpp"
#include "user/CRT.cpp"

using namespace UI;
using namespace Graphics;

FireScreen* screen;

bool UICallback(GUI& window, Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::PaintEvent:
	{
		FrameBuffer& frame = window.Frame;
		screen->Update();
		screen->Draw(frame);

		Buffer buff((uint8_t*)frame.Buffer, frame.Size());
		AssertSuccess(PaintWindow(window.GetHandle(), &buff));
		return true;
	}
	break;

	default:
		return false;
		break;
	}
}

int main(int argc, char** argv)
{
	ProcessInfo procInfo;
	GetProcessInfo(&procInfo);
	
	Rectangle frame;
	GetScreenRect(&frame);
	frame.Width /= 2;
	frame.Height /= 2;
	frame.X = frame.Width;
	frame.Y = frame.Height;

	screen = new FireScreen(frame.Height, frame.Width);
	screen->Initialize();

	WindowStyle style = {};
	style.IsBordered = true;

	GUI gui("Fire", frame, style, UICallback);
	gui.Initialize();

	gui.Run();

	return 0;
}
