#include "doomkeys.h"

#include "doomgeneric.h"

#include "core_crt/ctype.h"
#include "core_crt/string.h"
#include <stdio.h>

#include "user/MetalOS.h"
#include "user/MetalOS.UI.h"
#include "Assert.h"

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

using namespace Graphics;

static unsigned char convertToDoomKey(unsigned char key)
{
	switch (key)
	{
	case VK_RETURN:
		key = KEY_ENTER;
		break;
	case VK_ESCAPE:
		key = KEY_ESCAPE;
		break;
	case VK_LEFT:
		key = KEY_LEFTARROW;
		break;
	case VK_RIGHT:
		key = KEY_RIGHTARROW;
		break;
	case VK_UP:
		key = KEY_UPARROW;
		break;
	case VK_DOWN:
		key = KEY_DOWNARROW;
		break;
	case VK_CONTROL:
		key = KEY_FIRE;
		break;
	case VK_SPACE:
		key = KEY_USE;
		break;
	case VK_SHIFT:
		key = KEY_RSHIFT;
		break;
	default:
		key = tolower(key);
		break;
	}

	return key;
}

static void addKeyToQueue(int pressed, unsigned char keyCode)
{
	unsigned char key = convertToDoomKey(keyCode);

	unsigned short keyData = (pressed << 8) | key;

	s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
	s_KeyQueueWriteIndex++;
	s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

HWindow window;

static void ProcessMessage(const struct Message &message)
{
	switch (message.Header.MessageType)
	{
		//PaintEvent needs no handling: Doom renders straight into DG_ScreenBuffer,
		//which points at the window's shared surface, and the WM composites it.
	case MessageType::KeyEvent:
		addKeyToQueue(message.KeyEvent.Flags.Pressed, (unsigned char)message.KeyEvent.Key);
		break;
	}
}

extern "C"
{
	void DG_Init()
	{
		printf("DG_Init\n");

		//Create a window at Doom's fixed internal resolution so its shared surface is
		//exactly DG_BufferSize, then render zero-copy directly into that surface.
		Rectangle screen = {};
		GetScreenRect(&screen);

		Rectangle frame = {};
		frame.Width = DOOMGENERIC_RESX;
		frame.Height = DOOMGENERIC_RESY;
		frame.X = screen.Width > frame.Width ? (screen.Width - frame.Width) / 2 : 0;
		frame.Y = screen.Height > frame.Height ? (screen.Height - frame.Height) / 2 : 0;

		AllocWindow(&window, &frame, "Doom", false);
		DebugPrintf("Doom window: (0x%x,0x%x) 0x%x x 0x%x\n", frame.X, frame.Y, frame.Width, frame.Height);

		//Point Doom's screen buffer at the window surface (replaces the malloc done in
		//doomgeneric_Create); the WM composites it on its next pass.
		free(DG_ScreenBuffer);
		DG_ScreenBuffer = (pixel_t*)GetWindowSurface(window);
		Assert(DG_ScreenBuffer);

		memset(s_KeyQueue, 0, KEYQUEUE_SIZE * sizeof(unsigned short));
	}

	void DG_DrawFrame()
	{
		struct Message message;
		memset(&message, 0, sizeof(struct Message));
		while (PeekMessage(&message) == SyscallResult::Success)
		{
			ProcessMessage(message);
		}
	}

	void DG_SleepMs(uint32_t ms)
	{
		Sleep(ms);
	}

	uint32_t DG_GetTicksMs()
	{
		return (uint32_t)GetTickCount();
	}

	int DG_GetKey(int *pressed, unsigned char *doomKey)
	{
		if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
		{
			// key queue is empty
			return 0;
		}
		else
		{
			unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
			s_KeyQueueReadIndex++;
			s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

			*pressed = keyData >> 8;
			*doomKey = keyData & 0xFF;

			return 1;
		}
	}

	void DG_SetWindowTitle(const char *title)
	{
		//Not implemented
	}

	int main(int argc, char **argv)
	{
		doomgeneric_Create(argc, argv);

		while (1)
		{
			doomgeneric_Tick();
		}

		return 0;
	}
}
