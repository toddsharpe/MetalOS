#include <user/new>

#include <shared/MetalOS.Types.h>
#include <user/MetalOS.h>
#include <user/MetalOS.UI.h>

#include "Matrix.h"
#include "FireScreen.h"

#include <user/lockit>

FireScreen* screen;

bool UICallback(Window& window, Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::PaintEvent:
	{
		//Draw what we have
		ReadOnlyBuffer buffer = {};
		buffer.Data = screen->GetBuffer();
		buffer.Length = screen->GetSize();
		window.Paint(buffer);

		//Kick off update
		screen->Update();
		screen->Draw();
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

	//Change location based on proc id
	//int x = (procInfo.Id - 1) % 2;
	//int y = (procInfo.Id - 1) / 2;

	//frame.X = x * frame.Width / 2;
	//frame.Y = y * frame.Height / 2;
	//frame.Width = frame.Width / 2;
	//frame.Height = frame.Height / 2;

	screen = new FireScreen(frame.Height, frame.Width);
	screen->Initialize();
	screen->Draw();

	WindowStyle style = {};
	style.IsBordered = false;

	Window window("Fire", frame, style, UICallback);
	window.Initialize();

	window.Run();
}


