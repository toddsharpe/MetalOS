#include "user/MetalOS.h"
#include "user/MetalOS.UI.h"
#include "user/GUI.h"
#include "user/CRT.cpp"

using namespace Graphics;
using namespace UI;

bool UICallback(GUI& gui, Message& message)
{
	return false;
}

void B7Pressed(Button& button)
{
	DebugPrintf("B7 Clicked\n");
}

int main(int argc, char** argv)
{
	ProcessInfo procInfo;
	GetProcessInfo(&procInfo);

	DebugPrint("Hi From calc!\n");
	
	size_t pos = 25 + 50 * (procInfo.Id - 1);
	Rectangle rectangle = { pos, pos, 100, 200 };

	GUI gui("Calc", rectangle, UICallback);
	gui.Initialize();

	Button b7("7", { 5, 25, 20, 20 });
	gui.Children.Add(&b7);
	b7.Click = &B7Pressed;

	Button b8("8", { 30, 25, 20, 20 });
	gui.Children.Add(&b8);

	Button b9("9", { 55, 25, 20, 20 });
	gui.Children.Add(&b9);

	Button b4("4", { 5, 50, 20, 20 });
	gui.Children.Add(&b4);

	Button b5("5", { 30, 50, 20, 20 });
	gui.Children.Add(&b5);

	Button b6("6", { 55, 50, 20, 20 });
	gui.Children.Add(&b6);

	Button b1("1", { 5, 75, 20, 20 });
	gui.Children.Add(&b1);

	Button b2("2", { 30, 75, 20, 20 });
	gui.Children.Add(&b2);

	Button b3("3", { 55, 75, 20, 20 });
	gui.Children.Add(&b3);

	Label l1("HI!", {5, 175, 20, 20});
	gui.Children.Add(&l1);

	gui.Run();

	return 0;
}
