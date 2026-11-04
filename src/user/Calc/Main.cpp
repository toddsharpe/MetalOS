#include "user/MetalOS.h"
#include "user/GUI.h"
#include "UI/Button.h"
#include "UI/Label.h"
#include "user/CRT.cpp"
#include <stdio.h>

using namespace Graphics;
using namespace UI;

namespace
{
	//The display fits 9 characters, which also keeps every product inside an int64_t
	constexpr int64_t MaxValue = 999999999;
	constexpr int64_t MinValue = -99999999;

	Label* g_display;
	int64_t g_total;
	int64_t g_entry;
	bool g_typing;
	char g_pending = '=';

	void Show(const int64_t value)
	{
		char text[24];
		const int length = snprintf(text, sizeof(text), "%lld", value);
		g_display->Text.Set(CString(text, (size_t)length));
	}

	void Reset()
	{
		g_total = 0;
		g_entry = 0;
		g_typing = false;
		g_pending = '=';
	}

	//Folds the typed number into the total with the pending operator, failing on a divide by zero or a result too long to show
	bool Apply()
	{
		switch (g_pending)
		{
		case '+':
			g_total += g_entry;
			break;

		case '-':
			g_total -= g_entry;
			break;

		case '*':
			g_total *= g_entry;
			break;

		case '/':
			if (g_entry == 0)
				return false;
			g_total /= g_entry;
			break;

		default:
			g_total = g_entry;
			break;
		}

		return g_total >= MinValue && g_total <= MaxValue;
	}

	void KeyClicked(Button& button)
	{
		const char key = button.Text.c_str()[0];
		if (key >= '0' && key <= '9')
		{
			if (!g_typing)
			{
				g_entry = 0;
				g_typing = true;
			}

			if (g_entry <= MaxValue / 10)
				g_entry = g_entry * 10 + (key - '0');
			Show(g_entry);
			return;
		}

		if (key == 'C')
		{
			Reset();
			Show(0);
			return;
		}

		//An operator or = finishes the number being typed, and an operator with nothing typed replaces the pending one
		if (g_typing && !Apply())
		{
			Reset();
			g_display->Text.Set(CString("Error"));
			return;
		}

		g_typing = false;
		g_pending = key;
		Show(g_total);
	}

	constexpr Rectangle Key(const int32_t column, const int32_t row)
	{
		return { 5 + 25 * column, 25 + 25 * row, 20, 20 };
	}
}

bool UICallback(GUI& gui, Message& message)
{
	return false;
}

int main(int argc, char** argv)
{
	ProcessInfo procInfo;
	GetProcessInfo(&procInfo);

	DebugPrint("Hi From calc!\n");

	size_t pos = 25 + 50 * (procInfo.Id - 1);
	Rectangle rectangle = { pos, pos, 105, 150 };

	GUI gui("Calc", rectangle, UICallback);
	gui.Initialize();

	Button keys[] =
	{
		{ "7", Key(0, 0) }, { "8", Key(1, 0) }, { "9", Key(2, 0) }, { "/", Key(3, 0) },
		{ "4", Key(0, 1) }, { "5", Key(1, 1) }, { "6", Key(2, 1) }, { "*", Key(3, 1) },
		{ "1", Key(0, 2) }, { "2", Key(1, 2) }, { "3", Key(2, 2) }, { "-", Key(3, 2) },
		{ "C", Key(0, 3) }, { "0", Key(1, 3) }, { "=", Key(2, 3) }, { "+", Key(3, 3) },
	};

	for (Button& key : keys)
	{
		key.Click = &KeyClicked;
		gui.Children.Add(&key);
	}

	Label display("0", { 5, 125, 95, 20 });
	g_display = &display;
	gui.Children.Add(&display);

	gui.Run();

	return 0;
}
