#include <MetalOS.h>
#include <stdio.h>
#include <string.h>

void ProcessMessage(const Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::MessageTypeKeyEvent:
		DebugPrintf("Key: %c\n", message.KeyEvent.Key);
		break;
	}
}

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

int main(int argc, char** argv)
{
	SystemInfo sysInfo = { 0 };
	GetSystemInfo(&sysInfo);
	DebugPrintf("PageSize: 0x%x\n", sysInfo.PageSize);

	ProcessInfo info = { 0 };
	GetProcessInfo(&info);
	DebugPrintf("ProcessID: 0x%x\n", info.Id);

	Message message;
	memset(&message, 0, sizeof(Message));
	//while (WaitForMessages(NULL))
	//{
		//GetMessage(&message);
	//}
}
