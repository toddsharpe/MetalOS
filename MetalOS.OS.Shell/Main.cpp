#include <MetalOS.h>
#include <stdio.h>
#include <string.h>

void DebugPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[255];
	int retval = crt_vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);

	DebugPrint(buffer);
}

void ProcessMessage(const Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::MessageTypeKeyEvent:
		DebugPrintf("Key: %c\n", message.KeyEvent.Key);
		break;

	default:
		DebugPrintf("Event: 0x%x\n", message.Header.MessageType);
		break;
	}
}

int main(int argc, char** argv)
{
	SystemInfo sysInfo = { 0 };
	GetSystemInfo(&sysInfo);
	DebugPrintf("PageSize: 0x%x\n", sysInfo.PageSize);

	ProcessInfo info = { 0 };
	GetProcessInfo(&info);
	DebugPrintf("ProcessID: 0x%x\n", info.Id);

	Handle h = CreateWindow("Shell");

	Rectangle rect = { 0 };
	GetWindowRect(h, &rect);
	DebugPrintf("Rect: (0x%x,0x%x) x (0x%x,0x%x)\n",
		rect.P1.X, rect.P1.Y, rect.P2.X, rect.P2.Y);

	Message message;
	memset(&message, 0, sizeof(Message));
	while (GetMessage(&message) == SystemCallResult::Success)
	{
		ProcessMessage(message);
	}
}
