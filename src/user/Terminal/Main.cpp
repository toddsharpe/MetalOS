#include "user/MetalOS.h"
#include "user/MetalOS.UI.h"
#include "user/GUI.h"
#include "UI/Console.h"
#include "Lib/String.h"
#include "Assert.h"
#include "user/CRT.cpp"

using namespace Graphics;
using namespace UI;

namespace
{
	Console console({ 5, 25, 790, 370 });

	static constexpr CString Prompt = "> ";

	//Command lines are copied into 128 bytes by the kernel; the ".exe" suffix must still fit
	static constexpr size_t MaxCommandLength = 100;
	StaticString<MaxCommandLength + 2> command;

	//The running child: the read end of its combined stdout/stderr pipe, and its process handle
	HFile childOutput = nullptr;
	HProcess child = nullptr;

	//Bounds one repaint's worth of draining, so a child that never stops writing can't starve the UI
	static constexpr size_t MaxDrainPerPaint = 4096;

	void ShowPrompt()
	{
		console.Text.Write(Prompt);
	}

	//Prompt and command stay on one line, so Backspace never has to cross a wrapped line
	size_t CommandLimit()
	{
		const size_t columns = console.Text.Columns();
		const size_t fit = columns > Prompt.Length + 1 ? columns - Prompt.Length - 1 : 0;
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

	//Starts the command with stdout and stderr on one pipe; false if it couldn't be started
	bool Launch(const CString& commandLine)
	{
		HFile inRead = nullptr;
		HFile inWrite = nullptr;
		if (CreatePipe(&inRead, &inWrite) != SyscallResult::Success)
			return false;

		HFile outRead = nullptr;
		HFile outWrite = nullptr;
		if (CreatePipe(&outRead, &outWrite) != SyscallResult::Success)
		{
			CloseHandle(inRead);
			CloseHandle(inWrite);
			return false;
		}

		//Sharing one pipe keeps output in order, and an unread stderr can't fill up and stall the child
		CreateProcessArgs args = {};
		args.StdInput = inRead;
		args.StdOutput = outWrite;
		args.StdError = outWrite;

		CreateProcessResult result = {};
		const SyscallResult status = CreateProcess(commandLine.c_str(), &args, &result);

		//The child holds its own ends now; closing the stdin write end gives it end of file
		CloseHandle(inRead);
		CloseHandle(inWrite);
		CloseHandle(outWrite);

		if (status != SyscallResult::Success)
		{
			CloseHandle(outRead);
			return false;
		}

		childOutput = outRead;
		child = result.Process;
		return true;
	}

	//Moves what the child has written into the console without blocking; true once it's done and drained
	bool DrainOutput()
	{
		size_t drained = 0;
		while (drained < MaxDrainPerPaint)
		{
			size_t available = 0;
			const SyscallResult status = PeekNamedPipe(childOutput, &available);
			if (available == 0)
				return status != SyscallResult::Success;

			//ReadFile on a pipe waits for the full count, so never ask for more than is there
			char buffer[256];
			const size_t count = available < sizeof(buffer) ? available : sizeof(buffer);
			size_t read = 0;
			ReadFile(childOutput, buffer, count, &read);
			if (read == 0)
				return true;

			console.Text.Write(CString(buffer, read));
			drained += read;
		}
		return false;
	}

	void FinishCommand()
	{
		CloseHandle(childOutput);
		childOutput = nullptr;
		if (child)
			CloseHandle(child);
		child = nullptr;

		if (console.Text.CurrentLength() != 0)
			console.Text.Write('\n');
		ShowPrompt();
	}

	void RunCommand()
	{
		console.Text.Write('\n');
		if (command.Count() == 0)
		{
			ShowPrompt();
			return;
		}

		StaticString<MaxCommandLength + 8> line;
		BuildCommandLine(command, line);
		command.Clear();

		if (!Launch(line))
		{
			console.Text.Write(line);
			console.Text.Write(": could not run\n");
			ShowPrompt();
		}
	}

	void OnKey(const KeyEvent& key)
	{
		//Input is ignored while a command runs (it has no stdin to send it to)
		if (!key.Flags.Pressed || childOutput)
			return;

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
			return false;

		default:
			return false;
	}
}

int main(int argc, char** argv)
{
	DebugPrint("Hi From terminal!\n");

	Rectangle rectangle = { 100, 100, 800, 400 };

	WindowStyle style = {};
	style.IsBordered = true;

	GUI gui("Terminal", rectangle, style, UICallback);
	gui.Initialize();

	console.Background = Colors::Black;
	console.Foreground = Colors::White;
	console.Text.Write("MetalOS Terminal.\nTry typing doom\n");
	ShowPrompt();
	gui.Children.Add(&console);

	gui.Run();

	return 0;
}
