#include "user/MetalOS.h"
#include "user/MetalOS.UI.h"
#include "user/GUI.h"
#include <stdlib.h>
#include "Assert.h"

int __cdecl _purecall(void)
{
	Assert(false);
	return 0;
}

using namespace Graphics;
using namespace UI;

HFile stdIn;
HFile stdOut;
HFile stdError;

Label label("MetalOS Terminal.\nTry typing doom.exe\n>", { 5, 25, 790, 370 });

void LaunchProcess(const CString& cmd)
{
	CreateProcessArgs args = {};
	AssertSuccess(CreatePipe(&args.StdInput, &stdIn));
	AssertSuccess(CreatePipe(&stdOut, &args.StdOutput));
	AssertSuccess(CreatePipe(&stdError, &args.StdError));

	CreateProcessResult result = {};
	AssertSuccess(CreateProcess(cmd.c_str(), &args, &result));
	CloseHandle(args.StdInput);
	CloseHandle(args.StdOutput);
	CloseHandle(args.StdError);
}

bool ReadStdOut()
{
	size_t bytesAvailable = 0;
	SyscallResult result = PeekNamedPipe(stdOut, &bytesAvailable);
	if (result != SyscallResult::BrokenPipe)
		AssertSuccess(result);
	
	char* buffer = (char*)malloc(bytesAvailable);
	size_t read = 0;
	result = ReadFile(stdOut, buffer, bytesAvailable, &read);
	if (result != SyscallResult::BrokenPipe)
		AssertSuccess(result);

	//Append
	label.Text.Append(CString(buffer, bytesAvailable));

	return result == SyscallResult::BrokenPipe;
}

StaticString<1024> command;

HEvent startEvent;
HEvent endEvent;

enum class State
{
	//Waiting for user input
	Prompt,

	//Start process
	CreateProcess,

	//Read pipe as more info is available
	ReadStdout,

	//Finished
	Finished
};

uint32_t TerminalThread(void* arg)
{
	State state = State::Prompt;
	while (true)
	{
		switch (state)
		{
			case State::Prompt:
			{
				//Wait for start
				WaitStatus status;
				AssertSuccess(WaitForSingleObject(startEvent, UINT32_MAX, &status));
				state = State::CreateProcess;
			}
			break;

			case State::CreateProcess:
			{
				LaunchProcess(command);
				state = State::ReadStdout;
			}
			break;

			case State::ReadStdout:
			{
				size_t bytesAvailable = 0;
				SyscallResult result = PeekNamedPipe(stdOut, &bytesAvailable);
				if (result != SyscallResult::BrokenPipe)
					AssertSuccess(result);
				const bool isBroken = result == SyscallResult::BrokenPipe;

				if (!bytesAvailable)
				{
					if (isBroken)
						state = State::Finished;
					else
						Sleep(100);
					
					continue;
				}

				state = ReadStdOut() ? State::Finished : State::ReadStdout;
			}
			break;

			case State::Finished:
			{
				CloseHandle(stdIn);
				CloseHandle(stdOut);
				CloseHandle(stdError);

				AssertSuccess(SetEvent(endEvent));
				state = State::Prompt;
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
		if (message.KeyEvent.Flags.Pressed)
		{
			if (message.KeyEvent.Key == VK_RETURN)
			{
				if (command != "")
				{
					//Launch process
					label.Text.Append('\n');
					AssertSuccess(SetEvent(startEvent));

					//Wait for it to finish
					WaitStatus status;
					AssertSuccess(WaitForSingleObject(endEvent, UINT32_MAX, &status));
				}
				label.Text.Append("\n>");
				command.Set("");
			}
			else if (message.KeyEvent.Key == VK_BACK)
			{
				if (command.Count() == 0)
					break;
				command.Pop();
				label.Text.Pop();
			}
			else
			{
				char c = tolower((char)message.KeyEvent.Key);
				command.Append(c);
				label.Text.Append(c);
			}
		}
		break;

	default:
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	ProcessInfo procInfo;
	GetProcessInfo(&procInfo);

	DebugPrint("Hi From terminal!\n");

	Rectangle rectangle = { 100, 100, 800, 400 };

	WindowStyle style = {};
	style.IsBordered = true;

	GUI gui("Terminal", rectangle, style, UICallback);
	gui.Initialize();

	label.Background = Colors::Black;
	label.Foreground = Colors::White;
	gui.Children.Add(&label);

	AssertSuccess(CreateEvent(&startEvent, false, false));
	AssertSuccess(CreateEvent(&endEvent, false, false));

	CreateThread(0, TerminalThread, nullptr);
	gui.Run();

	return 0;
}

int atexit(void(__cdecl* func)(void))
{
	return 0;
}
