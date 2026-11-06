#include "user/MetalOS.h"
#include "user/GUI.h"
#include "UI/Console.h"
#include "Lib/String.h"
#include "Graphics/Theme.h"
#include "Graphics/Icons.h"
#include "Assert.h"
#include "user/CRT.cpp"

using namespace Graphics;
using namespace UI;

namespace
{
	constexpr Rectangle StartRect = { 160, 120, 1120, 620 };

	Console console({ 0, 0, StartRect.Width, StartRect.Height });

	static constexpr CString PromptName = "metal";
	static constexpr CString PromptMark = "> ";

	//Command lines are copied into 128 bytes by the kernel; the ".exe" suffix must still fit
	static constexpr size_t MaxCommandLength = 100;
	StaticString<MaxCommandLength + 2> command;

	//Earlier commands, oldest first, for Up and Down; historyIndex == historyCount is the line being typed
	static constexpr size_t HistorySize = 16;
	StaticString<MaxCommandLength + 2> history[HistorySize];
	size_t historyCount = 0;
	size_t historyIndex = 0;

	//The running child: the read ends of its stdout and stderr pipes, and its process handle
	HFile childOutput = nullptr;
	HFile childErrors = nullptr;
	HProcess child = nullptr;

	bool ctrlDown = false;

	//Bounds one repaint's worth of draining, so a child that never stops writing can't starve the UI
	static constexpr size_t MaxDrainPerPaint = 4096;

	void Write(const CString& text, const ConsoleAttribute attribute)
	{
		console.Text.SetAttribute(attribute);
		console.Text.Write(text);
		console.Text.SetAttribute(ConsoleAttribute::Normal);
	}

	void ShowPrompt()
	{
		Write(PromptName, ConsoleAttribute::Prompt);
		Write(PromptMark, ConsoleAttribute::Punctuation);
	}

	//Prompt and command stay on one line, so Backspace never has to cross a wrapped line
	size_t CommandLimit()
	{
		const size_t columns = console.Text.Columns();
		const size_t prompt = PromptName.Length + PromptMark.Length;
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
	void BuildCommandLine(const CString& typed, StaticString<MaxCommandLength + 8>& line)
	{
		size_t nameLength = 0;
		while (nameLength < typed.Length && typed[nameLength] != ' ')
			nameLength++;

		bool hasExtension = false;
		for (size_t i = 0; i < nameLength; i++)
			hasExtension |= typed[i] == '.';

		line.Append(CString(typed.c_str(), nameLength));
		if (!hasExtension)
			line.Append(".exe");
		line.Append(CString(typed.c_str() + nameLength, typed.Length - nameLength));
	}

	//Starts the command with its own stdout and stderr pipes, so its errors can show in red; false if it couldn't be started
	bool Launch(const CString& commandLine)
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
		const SyscallResult status = CreateProcess(commandLine.c_str(), &args, &result);

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

			Write(CString(buffer, read), attribute);
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

	void Remember(const CString& line)
	{
		if (historyCount == HistorySize)
		{
			for (size_t i = 1; i < HistorySize; i++)
			{
				history[i - 1].Clear();
				history[i - 1].Append(history[i]);
			}
			historyCount--;
		}
		history[historyCount].Clear();
		history[historyCount].Append(line);
		historyCount++;
		historyIndex = historyCount;
	}

	//Swaps the line being typed for another (from the history), on screen too
	void ReplaceCommand(const CString& text)
	{
		while (command.Count() != 0)
		{
			command.Pop();
			console.Text.Backspace();
		}

		const size_t limit = CommandLimit();
		for (size_t i = 0; i < text.Length && command.Count() < limit; i++)
		{
			command.Append(text[i]);
			console.Text.Write(text[i]);
		}
	}

	void ShowHelp()
	{
		Write("Run: ", ConsoleAttribute::Dim);
		Write("hello ps time calc fire doom dhcp ping nettest crash\n", ConsoleAttribute::Normal);
		Write("Built in: ", ConsoleAttribute::Dim);
		Write("help clear\n", ConsoleAttribute::Normal);
		Write("Keys: ", ConsoleAttribute::Dim);
		Write("Up/Down history, PgUp/PgDn scroll, Ctrl+L clear\n", ConsoleAttribute::Normal);
	}

	void ClearScreen()
	{
		console.Text.Clear();
		console.Scroll = 0;
		ShowPrompt();
		console.Text.Write(command);
	}

	void RunCommand()
	{
		console.Text.Write('\n');
		if (command.Count() == 0)
		{
			ShowPrompt();
			return;
		}

		Remember(command);
		if (CString(command) == "help")
		{
			command.Clear();
			ShowHelp();
			ShowPrompt();
			return;
		}
		if (CString(command) == "clear")
		{
			command.Clear();
			ClearScreen();
			return;
		}

		StaticString<MaxCommandLength + 8> line;
		BuildCommandLine(command, line);
		command.Clear();

		if (!Launch(line))
		{
			Write(line, ConsoleAttribute::Error);
			Write(": could not run\n", ConsoleAttribute::Error);
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
				if (command.Count() != 0)
				{
					command.Pop();
					console.Text.Backspace();
				}
				break;

			case VK_UP:
				if (historyIndex > 0)
				{
					historyIndex--;
					ReplaceCommand(history[historyIndex]);
				}
				break;

			case VK_DOWN:
				if (historyIndex < historyCount)
				{
					historyIndex++;
					ReplaceCommand(historyIndex == historyCount ? CString("") : CString(history[historyIndex]));
				}
				break;

			default:
			{
				const char c = ToChar(key.Key);
				if (c != '\0' && command.Count() < CommandLimit())
				{
					command.Append(c);
					console.Text.Write(c);
				}
			}
			break;
		}
	}
}

bool UICallback(GUI& gui, Message& message)
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
	DebugPrint("Hi From terminal!\n");

	GUI gui("Terminal", StartRect, UICallback, WindowStyle::Resizable, &Icons::Terminal);
	gui.Initialize();
	gui.Background = Theme::IronDeep;

	Write("MetalOS Terminal", ConsoleAttribute::Bright);
	Write("  type help for commands\n", ConsoleAttribute::Dim);
	ShowPrompt();
	gui.Children.Add(&console);

	gui.Run();

	return 0;
}
