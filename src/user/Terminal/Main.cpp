#include <user/MetalOS.h>
#include <user/MetalOS.UI.h>
#include "Assert.h"

#include <string>
#include <vector>

using namespace Graphics;
using namespace UI;

HFile stdIn;
HFile stdOut;
HFile stdError;

Label* label;

void LaunchProcess(const std::string& cmd)
{
	CreateProcessArgs args = {};
	AssertSuccess(CreatePipe(args.StdInput, stdIn));
	AssertSuccess(CreatePipe(stdOut, args.StdOutput));
	AssertSuccess(CreatePipe(stdError, args.StdError));

	CreateProcessResult result = {};
	AssertSuccess(CreateProcess(cmd.c_str(), &args, &result));
	CloseHandle(args.StdInput);
	CloseHandle(args.StdOutput);
	CloseHandle(args.StdError);
}

bool ReadStdOut()
{
	PipeInfo info = {};
	AssertSuccess(GetPipeInfo(stdOut, info));
	
	char* buffer = (char*)malloc(info.BytesAvailable);
	size_t read;

	SystemCallResult result = ReadFile(stdOut, buffer, info.BytesAvailable, &read);
	if (result != SystemCallResult::BrokenPipe)
		AssertSuccess(result);

	size_t index = 0;
	while (index < read)
	{
		char* start = (buffer + index);
		index += strlen(start) + 1;
		label->Text += start;
	}
	free(buffer);

	return info.IsBroken;
}

std::string command;

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

size_t TerminalThread(void* arg)
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
				AssertSuccess(WaitForSingleObject(startEvent, UINT32_MAX, status));
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
				PipeInfo info = {};
				AssertSuccess(GetPipeInfo(stdOut, info));
				

				if (!info.BytesAvailable)
				{
					if (info.IsBroken)
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
					label->Text += "\n";
					AssertSuccess(SetEvent(startEvent));

					//Wait for it to finish
					WaitStatus status;
					AssertSuccess(WaitForSingleObject(endEvent, UINT32_MAX, status));
				}
				label->Text += "\n>";
				command = "";
			}
			else if (message.KeyEvent.Key == VK_BACK)
			{
				if (command.length() == 0)
					break;
				command.pop_back();
				label->Text.pop_back();
			}
			else
			{
				char c = tolower((char)message.KeyEvent.Key);
				command += c;
				label->Text += c;
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
	GetProcessInfo(procInfo);

	DebugPrint("Hi From terminal!\n");

	Rectangle rectangle = { 100, 100, 400, 400 };

	WindowStyle style = {};
	style.IsBordered = true;

	GUI gui("Terminal", rectangle, style, UICallback);
	gui.Initialize();

	label = new Label("MetalOS Terminal.\nTry typing doom.exe\n>", { 5, 25, 390, 370 });
	label->Background = Colors::Black;
	label->Foreground = Colors::White;
	gui.Children.push_back(label);

	AssertSuccess(CreateEvent(startEvent, false, false));
	AssertSuccess(CreateEvent(endEvent, false, false));

	CreateThread(0, TerminalThread, nullptr);
	gui.Run();
}

int atexit(void(__cdecl* func)(void))
{
	return 0;
}
