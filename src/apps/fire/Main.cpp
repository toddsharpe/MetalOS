#include <user/MetalOS.h>
#include <user/MetalOS.UI.h>

#include "FireScreen.h"

#include <user/new>

using namespace UI;
using namespace Graphics;

FireScreen* screen;

bool UICallback(GUI& window, Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::PaintEvent:
	{
		FrameBuffer& frame = window.GetFrameBuffer();
		screen->Update();
		screen->Draw(frame);
		AssertSuccess(PaintWindow(window.GetHandle(), { frame.GetBuffer(), frame.Size()}));
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
	GetProcessInfo(procInfo);
	
	Rectangle frame;
	GetScreenRect(frame);
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
}
