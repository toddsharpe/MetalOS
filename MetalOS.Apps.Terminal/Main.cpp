#include <msvc.h>
#include "new"
#include <string>

#include <MetalOS.h>
#include <MetalOS.UI.h>

#include <vector>
#include <lockit>
#include <msvc.cpp>

std::string command;
Label* label;

HFile stdIn;
HFile stdOut;
HFile stdError;

HEvent event;

enum class State
{
	//Waiting for user input
	Prompt,

	//Start process
	StartProcess,

	//Read pipe as more info is available
	WaitForProcess,

	//Read whats left in the pipes, back to prompt
	FinalRead,
};

int atexit(void(__cdecl* func)(void))
{
	return 0;
}

//todo: should launch suspended?
void LaunchProcess(const std::string command)
{
	CreateProcessArgs args = {};
	AssertSuccess(CreatePipe(args.StdInput, stdIn));
	AssertSuccess(CreatePipe(stdOut, args.StdOutput));
	AssertSuccess(CreatePipe(stdError, args.StdError));

	CreateProcessResult result = {};
	AssertSuccess(CreateProcess(command.c_str(), &args, &result));
	CloseHandle(args.StdInput);
	CloseHandle(args.StdOutput);
	CloseHandle(args.StdError);

	AssertSuccess(SetEvent(event));

	WaitStatus status;
	AssertSuccess(WaitForSingleObject(result.Process, UINT32_MAX, status));
	DebugPrintf("WaitStatus: %d\n", status);
}

size_t TerminalThread(void* arg)
{
	DebugPrintf("TerminalThread\n");
	while (true)
	{
		WaitStatus status;
		AssertSuccess(WaitForSingleObject(event, UINT32_MAX, status))

		//DebugPrintf("waiting\n");
		//AssertSuccess(WaitForSingleObject(stdOut, 0xFFFFFFFF));
		//DebugPrintf("signaled\n");

		//WaitStatus status;
		//AssertSuccess(WaitForSingleObject(stdOut, INT32_MAX, status));
		//if (status == WaitStatus::)


		PipeInfo info = {};
		AssertSuccess(GetPipeInfo(stdOut, info));

		DebugPrintf("C: 0x%x\n", info.BytesAvailable);
		if (!info.BytesAvailable)
		{
			Sleep(500);
			continue;
		}

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
		DebugPrintBytes(buffer, read);
		free(buffer);

		if (info.IsBroken)
		{
			DebugPrintf("BrokenPipe\n");
			stdOut = nullptr;
			//reset event
			AssertSuccess(ResetEvent(event));
		}
	}
}

bool UICallback(Window& window, Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::KeyEvent:
		if (message.KeyEvent.Flags.Pressed)
		{
			if (message.KeyEvent.Key == VK_RETURN)
			{
				//Launch process
				label->Text += "\n";
				LaunchProcess(command);
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
				DebugPrintf("Char: %c 0x%x\n", message.KeyEvent.Key, message.KeyEvent.Key);
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

	Window window("Terminal", rectangle, style, UICallback);
	window.Initialize();

	label = new Label(">", { 5, 25, 390, 370 });
	label->Background = Colors::Black;
	label->Foreground = Colors::White;
	window.Children.push_back(label);

	AssertSuccess(CreateEvent(event, true, false));

	CreateThread(0, TerminalThread, nullptr);
	window.Run();
}
