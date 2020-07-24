#include <MetalOS.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Debug.h>

void ProcessMessage(const Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::KeyEvent:
		if (message.KeyEvent.Key >= 'A' && message.KeyEvent.Key <= 'Z')
			DebugPrintf("Key: %c - %d\n", message.KeyEvent.Key, message.KeyEvent.Flags.Pressed);
		else
			DebugPrintf("Key: 0x%x - %d\n", message.KeyEvent.Key, message.KeyEvent.Flags.Pressed);
		break;

	default:
		DebugPrintf("Event: 0x%x\n", message.Header.MessageType);
		break;
	}
}

int main(int argc, char** argv)
{
	SystemInfo sysInfo = { 0 };
	GetSystemInfo(sysInfo);
	DebugPrintf("PageSize: 0x%x\n", sysInfo.PageSize);

	ProcessInfo info = { 0 };
	GetProcessInfo(info);
	DebugPrintf("ProcessID: 0x%x\n", info.Id);

	Handle h = CreateWindow("Shell");

	Rectangle rect = { 0 };
	GetWindowRect(h, rect);
	DebugPrintf("Rect: (0x%x,0x%x) x (0x%x,0x%x)\n",
		rect.P1.X, rect.P1.Y, rect.P2.X, rect.P2.Y);

	Message message;
	memset(&message, 0, sizeof(struct Message));
	while (GetMessage(message) == SystemCallResult::Success)
	{
		ProcessMessage(message);
	}
}
