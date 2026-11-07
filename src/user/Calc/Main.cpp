#include "user/MetalOS.h"
#include "user/GUI.h"
#include "UI/Button.h"
#include "Graphics/Theme.h"
#include "Graphics/Icons.h"
#include <stdio.h>
#include <string.h>

using namespace Graphics;
using namespace UI;

namespace
{
	//The display fits 9 characters, which also keeps every product inside an int64_t
	static constexpr int64_t MaxValue = 999999999;
	static constexpr int64_t MinValue = -99999999;

	static constexpr int32_t Padding = 14;
	static constexpr uint32_t KeyWidth = 72;
	static constexpr uint32_t KeyHeight = 56;
	static constexpr int32_t KeyGap = 8;
	static constexpr uint32_t ReadoutHeight = 92;
	static constexpr uint32_t ClientWidth = 2 * Padding + 4 * KeyWidth + 3 * KeyGap;
	static constexpr uint32_t ClientHeight = 2 * Padding + ReadoutHeight + 12 + 4 * KeyHeight + 3 * KeyGap;

	int64_t g_total;
	int64_t g_entry;
	bool g_typing;
	char g_pending = '=';
	char g_value[24] = "0";
	char g_history[40] = "";

	//The number on a dark recessed panel, in big molten digits over unlit ones, with the sum so far on a small line above
	class Readout : public Control
	{
	public:
		Readout(const Rectangle& bounds) : Control(bounds)
		{
		}

		virtual void Draw(FrameBuffer& frame) override
		{
			Draw2D::DrawRectangle(frame, Theme::Well, Bounds);
			Draw2D::DrawOutline(frame, Bounds, Theme::Outline);
			Draw2D::DrawBevel(frame, { Bounds.X + 1, Bounds.Y + 1, Bounds.Width - 2, Bounds.Height - 2 }, Theme::Rgb(0x000000), Theme::Rgb(0x2C2F34));

			const int32_t right = Bounds.X + (int32_t)Bounds.Width - 10;
			const size_t historyLength = strlen(g_history);
			Draw2D::DrawPixelText(frame, { right - (int32_t)Draw2D::PixelTextWidth(historyLength, 1), Bounds.Y + 12 }, { g_history, historyLength }, Theme::Hint, 1);

			const int32_t digitsY = Bounds.Y + (int32_t)Bounds.Height - 14 - 32;
			const cstr_t unlit = Str("888888888");
			Draw2D::DrawPixelText(frame, { right - (int32_t)Draw2D::PixelTextWidth(unlit.Size, 4), digitsY }, unlit, Theme::Rgb(0x1D100A), 4);
			const size_t valueLength = strlen(g_value);
			Draw2D::DrawPixelText(frame, { right - (int32_t)Draw2D::PixelTextWidth(valueLength, 4), digitsY }, { g_value, valueLength }, Theme::Molten, 4);
		}
	};

	void Show(const int64_t value)
	{
		snprintf(g_value, sizeof(g_value), "%lld", value);
	}

	void Reset()
	{
		g_total = 0;
		g_entry = 0;
		g_typing = false;
		g_pending = '=';
		g_history[0] = '\0';
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

	//A key from the keypad or keyboard: a digit, an operator, '=', 'C' (clear) or 'B' (backspace)
	void Press(const char key)
	{
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

		if (key == 'B')
		{
			if (g_typing)
			{
				g_entry /= 10;
				Show(g_entry);
			}
			return;
		}

		//An operator or = finishes the number being typed, and an operator with nothing typed replaces the pending one
		const int64_t before = g_total;
		const char pending = g_pending;
		const int64_t entry = g_entry;
		const bool typed = g_typing;
		if (g_typing && !Apply())
		{
			Reset();
			snprintf(g_value, sizeof(g_value), "Error");
			return;
		}
		g_typing = false;

		if (key == '=')
		{
			if (typed && pending != '=')
				snprintf(g_history, sizeof(g_history), "%lld %c %lld =", before, pending, entry);
			else
				g_history[0] = '\0';
			g_pending = '=';
		}
		else
		{
			g_pending = key;
			snprintf(g_history, sizeof(g_history), "%lld %c", g_total, key);
		}
		Show(g_total);
	}

	void KeyClicked(Button& button)
	{
		Press(button.Text[0]);
	}

	//The calculator key a keyboard key stands for, or 0
	char KeyFor(const VirtualKey key)
	{
		if (key >= '0' && key <= '9')
			return (char)key;
		if (key >= VK_NUMPAD0 && key <= VK_NUMPAD9)
			return (char)('0' + (key - VK_NUMPAD0));

		switch (key)
		{
			case VK_ADD:
			case VK_OEM_PLUS:
				return '+';

			case VK_SUBTRACT:
			case VK_OEM_MINUS:
				return '-';

			case VK_MULTIPLY:
				return '*';

			case VK_DIVIDE:
			case VK_OEM_2:
				return '/';

			case VK_RETURN:
			case VK_OEM_1:
				return '=';

			case VK_ESCAPE:
				return 'C';

			case VK_BACK:
				return 'B';

			default:
				return 0;
		}
	}

	constexpr Rectangle Key(const int32_t column, const int32_t row)
	{
		return { Padding + (int32_t)(KeyWidth + KeyGap) * column, Padding + (int32_t)ReadoutHeight + 12 + (int32_t)(KeyHeight + KeyGap) * row, KeyWidth, KeyHeight };
	}

	Button keys[] =
	{
		{ Str("7"), Key(0, 0) }, { Str("8"), Key(1, 0) }, { Str("9"), Key(2, 0) }, { Str("/"), Key(3, 0), ButtonStyle::Operator },
		{ Str("4"), Key(0, 1) }, { Str("5"), Key(1, 1) }, { Str("6"), Key(2, 1) }, { Str("*"), Key(3, 1), ButtonStyle::Operator },
		{ Str("1"), Key(0, 2) }, { Str("2"), Key(1, 2) }, { Str("3"), Key(2, 2) }, { Str("-"), Key(3, 2), ButtonStyle::Operator },
		{ Str("C"), Key(0, 3), ButtonStyle::Danger }, { Str("0"), Key(1, 3) }, { Str("="), Key(2, 3), ButtonStyle::Primary }, { Str("+"), Key(3, 3), ButtonStyle::Operator },
	};

	//A typed key works its keypad button, which shows pressed while the key is held
	void OnKey(const KeyEvent& event)
	{
		const char key = KeyFor(event.Key);
		if (key == 0)
			return;

		for (Button& button : keys)
		{
			if (button.Text[0] == key)
				button.Pressed = event.Flags.Pressed;
		}
		if (event.Flags.Pressed)
			Press(key);
	}
}

static bool UICallback(GUI& gui, Message& message)
{
	UNUSED(gui);
	if (message.Header.MessageType == MessageType::KeyEvent)
	{
		OnKey(message.KeyEvent);
		return true;
	}
	return false;
}

int main(int argc, char** argv)
{
	UNUSED(argc);
	UNUSED(argv);
	ProcessInfo procInfo;
	GetProcessInfo(&procInfo);
	DebugPrint("Hi From calc!\n");

	//Each calculator opens a step down and right of the last
	const int32_t pos = 25 + 50 * (int32_t)(procInfo.Id - 1);
	const Rectangle rectangle = { pos, pos + Theme::FrameTop, ClientWidth, ClientHeight };

	GUI gui(Str("Calc"), rectangle, UICallback, WindowStyle::Framed, &Icons::Calc);
	gui.Initialize();

	for (Button& key : keys)
	{
		key.Click = &KeyClicked;
		gui.Children.Add(&key);
	}

	Readout readout({ Padding, Padding, ClientWidth - 2 * Padding, ReadoutHeight });
	gui.Children.Add(&readout);

	gui.Run();

	return 0;
}
