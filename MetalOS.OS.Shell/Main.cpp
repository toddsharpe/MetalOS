#include <MetalOS.h>
#include <stdio.h>
#include <string.h>

void ProcessMessage(const Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::MessageTypeKeyEvent:
		break;
	}
}

int main(int argc, char** argv)
{
	SystemInfo sysInfo = { 0 };
	GetSystemInfo(&sysInfo);

	ProcessInfo info = { 0 };
	GetProcessInfo(&info);

	Message message;
	while (WaitForMessages(NULL))
	{
		GetMessage(&message);
	}
}
