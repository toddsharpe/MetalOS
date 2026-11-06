#include "user/MetalOS.h"
#include "user/GUI.h"
#include "Graphics/Theme.h"
#include "Graphics/Icons.h"
#include "User/Fire/FireScreen.h"

using namespace Graphics;

static FireScreen* screen;

static bool UICallback(GUI& window, Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::PaintEvent:
	{
		FrameBuffer& frame = window.Frame;
		screen->Update();
		screen->Draw(frame);

		//Draw straight into the WM-provided shared surface; the WM composites it.
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
	//A quarter of the work area, framed, near the bottom-right corner
	Rectangle area;
	GetScreenRect(&area);
	const uint32_t outerWidth = area.Width / 2;
	const uint32_t outerHeight = area.Height / 2;
	Rectangle frame;
	frame.Width = outerWidth - 2 * Theme::FrameEdge;
	frame.Height = outerHeight - Theme::FrameTop - Theme::FrameEdge;
	frame.X = (int32_t)(area.Width - outerWidth) - 24 + Theme::FrameEdge;
	frame.Y = (int32_t)(area.Height - outerHeight) - 24 + Theme::FrameTop;

	screen = new FireScreen(frame.Height, frame.Width);
	screen->Initialize();

	GUI gui(Str("Fire"), frame, UICallback, WindowStyle::Framed, &Icons::Flame);
	gui.Initialize();

	gui.Run();

	return 0;
}
