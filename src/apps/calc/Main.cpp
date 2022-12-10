#include <user/MetalOS.h>
#include <user/MetalOS.UI.h>

#include <user/new>
#include <string>
#include <vector>

using namespace Graphics;
using namespace UI;

#define TEST_MODE 0

bool UICallback(GUI& gui, Message& message)
{
	return false;
}

#if TEST_MODE
//Test method to reposition window at 10hz
size_t MoveThread(void* arg)
{
	GUI* window = (GUI*)arg;
	
	Graphics::Rectangle rect;
	window->GetRectangle(rect);
	DebugPrintf("Rect: 0x%x 0x%x 0x%x 0x%x\n", rect.X, rect.Y, rect.Width, rect.Height);

	while (true)
	{
		rect.X++;
		rect.Y++;
		window->Move(rect);

		Sleep(1000 / 10);
	}
}
#endif

void B7Pressed(Button& button)
{
	DebugPrintf("B7 Clicked\n");
}

int main(int argc, char** argv)
{
	ProcessInfo procInfo;
	GetProcessInfo(procInfo);

	DebugPrint("Hi From calc!\n");
	
	size_t pos = 25 + 50 * (procInfo.Id - 1);
	Rectangle rectangle = { pos, pos, 100, 200 };
	
	WindowStyle style = {};
	style.IsBordered = true;

	GUI gui("Calc", rectangle, style, UICallback);
	gui.Initialize();

	Button b7("7", { 5, 25, 20, 20 });
	gui.Children.push_back(&b7);
	b7.Click = &B7Pressed;

	Button b8("8", { 30, 25, 20, 20 });
	gui.Children.push_back(&b8);

	Button b9("9", { 55, 25, 20, 20 });
	gui.Children.push_back(&b9);

	Button b4("4", { 5, 50, 20, 20 });
	gui.Children.push_back(&b4);

	Button b5("5", { 30, 50, 20, 20 });
	gui.Children.push_back(&b5);

	Button b6("6", { 55, 50, 20, 20 });
	gui.Children.push_back(&b6);

	Button b1("1", { 5, 75, 20, 20 });
	gui.Children.push_back(&b1);

	Button b2("2", { 30, 75, 20, 20 });
	gui.Children.push_back(&b2);

	Button b3("3", { 55, 75, 20, 20 });
	gui.Children.push_back(&b3);

	Label l1("HI!", {5, 175, 20, 20});
	gui.Children.push_back(&l1);

#if TEST_MODE
	CreateThread(0, MoveThread, (void*)&gui);
#endif
	gui.Run();
}
