#include "user/MetalOS.h"
#include "user/GUI.h"
#include "UI/Console.h"
#include "Lib/String.h"
#include "Graphics/Theme.h"
#include "Graphics/Icons.h"
#include "Assert.h"

using namespace Graphics;
using namespace UI;

namespace
{
	static constexpr Rectangle StartRect = { 160, 120, 1120, 620 };

	ConsoleText scrollback;
	Console console(scrollback, { 0, 0, StartRect.Width, StartRect.Height });

	static constexpr char PromptName[] = "metal";
	static constexpr char PromptMark[] = "> ";

	//Command lines are copied into 128 bytes by the kernel; the ".exe" suffix must still fit
	static constexpr size_t MaxCommandLength = 100;
	char command[MaxCommandLength + 2];
	size_t commandLength = 0;

	//Earlier commands, oldest first, for Up and Down; historyIndex == historyCount is the line being typed
	static constexpr size_t HistorySize = 16;
	char history[HistorySize][MaxCommandLength + 2];
	size_t historyCount = 0;
	size_t historyIndex = 0;

	//The running child: the read ends of its stdout and stderr pipes, and its process handle
	HFile childOutput = nullptr;
	HFile childErrors = nullptr;
	HProcess child = nullptr;

	bool ctrlDown = false;

	//Bounds one repaint's worth of draining, so a child that never stops writing can't starve the UI
	static constexpr size_t MaxDrainPerPaint = 4096;

	void Write(const cstr_t text, const ConsoleAttribute attribute)
	{
		console.Text.SetAttribute(attribute);
		console.Text.Write(text);
		console.Text.SetAttribute(ConsoleAttribute::Normal);
	}

	void ShowPrompt()
	{
		Write(Str(PromptName), ConsoleAttribute::Prompt);
		Write(Str(PromptMark), ConsoleAttribute::Punctuation);
	}

	//Prompt and command stay on one line, so Backspace never has to cross a wrapped line
	size_t CommandLimit()
	{
		const size_t columns = console.Text.Columns();
		const size_t prompt = Str(PromptName).Size + Str(PromptMark).Size;
		const size_t fit = columns > prompt + 1 ? columns - prompt - 1 : 0;
		return fit < MaxCommandLength ? fit : MaxCommandLength;
	}

	//The keyboard driver reports virtual keys, and there's no shift handling yet, so only these type text
	char ToChar(const VirtualKey key)
	{
		if (key >= 'A' && key <= 'Z')
			return (char)(key - 'A' + 'a');
		if ((key >= '0' && key <= '9') || key == ' ' || key == '.' || key == ',')
			return (char)key;

		switch (key)
		{
			case VK_OEM_MINUS:
				return '-';

			case VK_OEM_2:
				return '/';

			default:
				return '\0';
		}
	}

	//Copies the command, appending .exe to the program name when it has no extension (so "ps" runs ps.exe)
	void BuildCommandLine(const cstr_t typed, char (&line)[MaxCommandLength + 8])
	{
		size_t nameLength = 0;
		while (nameLength < typed.Size && typed[nameLength] != ' ')
			nameLength++;

		bool hasExtension = false;
		for (size_t i = 0; i < nameLength; i++)
			hasExtension |= typed[i] == '.';

		StrCopy(line, typed.Slice(0, nameLength));
		if (!hasExtension)
			StrAppend(line, Str(".exe"));
		StrAppend(line, typed.Slice(nameLength, typed.Size - nameLength));
	}

	//Adds a typed char to the command, or takes the last one off
	void PushCommand(const char c)
	{
		command[commandLength++] = c;
		command[commandLength] = '\0';
	}

	void PopCommand()
	{
		command[--commandLength] = '\0';
	}

	void ClearCommand()
	{
		commandLength = 0;
		command[0] = '\0';
	}

	//Starts the command with its own stdout and stderr pipes, so its errors can show in red; false if it couldn't be started
	bool Launch(const char* const commandLine)
	{
		HFile inRead = nullptr;
		HFile inWrite = nullptr;
		if (CreatePipe(&inRead, &inWrite) != SyscallResult::Success)
			return false;

		HFile outRead = nullptr;
		HFile outWrite = nullptr;
		HFile errRead = nullptr;
		HFile errWrite = nullptr;
		if (CreatePipe(&outRead, &outWrite) != SyscallResult::Success)
		{
			CloseHandle(inRead);
			CloseHandle(inWrite);
			return false;
		}
		if (CreatePipe(&errRead, &errWrite) != SyscallResult::Success)
		{
			CloseHandle(inRead);
			CloseHandle(inWrite);
			CloseHandle(outRead);
			CloseHandle(outWrite);
			return false;
		}

		CreateProcessArgs args = {};
		args.StdInput = inRead;
		args.StdOutput = outWrite;
		args.StdError = errWrite;

		CreateProcessResult result = {};
		const SyscallResult status = CreateProcess(commandLine, &args, &result);

		//The child holds its own ends now; closing the stdin write end gives it end of file
		CloseHandle(inRead);
		CloseHandle(inWrite);
		CloseHandle(outWrite);
		CloseHandle(errWrite);

		if (status != SyscallResult::Success)
		{
			CloseHandle(outRead);
			CloseHandle(errRead);
			return false;
		}

		childOutput = outRead;
		childErrors = errRead;
		child = result.Process;
		return true;
	}

	//Moves what one of the child's pipes holds into the console without blocking; true once that pipe is finished
	bool Drain(const HFile pipe, const ConsoleAttribute attribute, size_t& budget)
	{
		while (budget != 0)
		{
			size_t available = 0;
			const SyscallResult status = PeekNamedPipe(pipe, &available);
			if (available == 0)
				return status != SyscallResult::Success;

			//The pipe has data, so ReadFile returns at once with up to a buffer's worth
			char buffer[256];
			size_t read = 0;
			ReadFile(pipe, buffer, budget < sizeof(buffer) ? budget : sizeof(buffer), &read);
			if (read == 0)
				return true;

			Write({ buffer, read }, attribute);
			budget -= read;
		}
		return false;
	}

	//True once the child has finished both pipes; both are drained every paint, so neither can fill up and stall it
	bool DrainOutput()
	{
		size_t budget = MaxDrainPerPaint;
		const bool outputDone = Drain(childOutput, ConsoleAttribute::Normal, budget);
		const bool errorsDone = Drain(childErrors, ConsoleAttribute::Error, budget);
		return outputDone && errorsDone;
	}

	void FinishCommand()
	{
		CloseHandle(childOutput);
		CloseHandle(childErrors);
		childOutput = nullptr;
		childErrors = nullptr;
		if (child)
			CloseHandle(child);
		child = nullptr;

		if (console.Text.CurrentLength() != 0)
			console.Text.Write('\n');
		ShowPrompt();
	}

	void Remember(const cstr_t line)
	{
		if (historyCount == HistorySize)
		{
			for (size_t i = 1; i < HistorySize; i++)
				StrCopy(history[i - 1], Str(history[i]));
			historyCount--;
		}
		StrCopy(history[historyCount], line);
		historyCount++;
		historyIndex = historyCount;
	}

	//Swaps the line being typed for another (from the history), on screen too
	void ReplaceCommand(const cstr_t text)
	{
		while (commandLength != 0)
		{
			PopCommand();
			console.Text.Backspace();
		}

		const size_t limit = CommandLimit();
		for (size_t i = 0; i < text.Size && commandLength < limit; i++)
		{
			PushCommand(text[i]);
			console.Text.Write(text[i]);
		}
	}

	void ShowHelp()
	{
		Write(Str("Run: "), ConsoleAttribute::Dim);
		Write(Str("hello ps time calc fire doom dhcp ping nettest crash\n"), ConsoleAttribute::Normal);
		Write(Str("Built in: "), ConsoleAttribute::Dim);
		Write(Str("help clear\n"), ConsoleAttribute::Normal);
		Write(Str("Keys: "), ConsoleAttribute::Dim);
		Write(Str("Up/Down history, PgUp/PgDn scroll, Ctrl+L clear\n"), ConsoleAttribute::Normal);
	}

	void ClearScreen()
	{
		console.Text.Clear();
		console.Scroll = 0;
		ShowPrompt();
		console.Text.Write(Str(command));
	}

	void RunCommand()
	{
		console.Text.Write('\n');
		if (commandLength == 0)
		{
			ShowPrompt();
			return;
		}

		const cstr_t typed = { command, commandLength };
		Remember(typed);
		if (typed == Str("help"))
		{
			ClearCommand();
			ShowHelp();
			ShowPrompt();
			return;
		}
		if (typed == Str("clear"))
		{
			ClearCommand();
			ClearScreen();
			return;
		}

		char line[MaxCommandLength + 8];
		BuildCommandLine(typed, line);
		ClearCommand();

		if (!Launch(line))
		{
			Write(Str(line), ConsoleAttribute::Error);
			Write(Str(": could not run\n"), ConsoleAttribute::Error);
			ShowPrompt();
		}
	}

	void OnKey(const KeyEvent& key)
	{
		if (key.Key == VK_CONTROL)
		{
			ctrlDown = key.Flags.Pressed;
			return;
		}
		if (!key.Flags.Pressed)
			return;

		//Scrolling back works while a command runs too
		if (key.Key == VK_PRIOR || key.Key == VK_NEXT)
		{
			const int64_t page = console.Rows() > 1 ? (int64_t)console.Rows() - 1 : 1;
			console.ScrollBy(key.Key == VK_PRIOR ? page : -page);
			return;
		}

		//Other input is ignored while a command runs (it has no stdin to send it to)
		if (childOutput)
			return;

		console.Scroll = 0;
		if (ctrlDown && key.Key == 'L')
		{
			ClearScreen();
			return;
		}

		switch (key.Key)
		{
			case VK_RETURN:
				RunCommand();
				break;

			case VK_BACK:
				if (commandLength != 0)
				{
					PopCommand();
					console.Text.Backspace();
				}
				break;

			case VK_UP:
				if (historyIndex > 0)
				{
					historyIndex--;
					ReplaceCommand(Str(history[historyIndex]));
				}
				break;

			case VK_DOWN:
				if (historyIndex < historyCount)
				{
					historyIndex++;
					ReplaceCommand(historyIndex == historyCount ? cstr_t() : Str(history[historyIndex]));
				}
				break;

			default:
			{
				const char c = ToChar(key.Key);
				if (c != '\0' && commandLength < CommandLimit())
				{
					PushCommand(c);
					console.Text.Write(c);
				}
			}
			break;
		}
	}
}

static bool UICallback(GUI& gui, Message& message)
{
	switch (message.Header.MessageType)
	{
		case MessageType::KeyEvent:
			OnKey(message.KeyEvent);
			return true;

		case MessageType::PaintEvent:
			//The WM asks for a repaint every frame, which is also when a running command's output is collected
			if (childOutput && DrainOutput())
				FinishCommand();
			console.ShowCursor = childOutput == nullptr;
			return false;

		case MessageType::ResizeEvent:
			console.SetBounds({ 0, 0, gui.Frame.Width, gui.Frame.Height });
			return false;

		default:
			return false;
	}
}

int main(int argc, char** argv)
{
	UNUSED(argc);
	UNUSED(argv);
	DebugPrint("Hi From terminal!\n");

	GUI gui(Str("Terminal"), StartRect, UICallback, WindowStyle::Resizable, &Icons::Terminal);
	gui.Initialize();
	gui.Background = Theme::IronDeep;

	Write(Str("MetalOS Terminal"), ConsoleAttribute::Bright);
	Write(Str("  type help for commands\n"), ConsoleAttribute::Dim);
	ShowPrompt();
	gui.Children.Add(&console);

	gui.Run();

	return 0;
}
