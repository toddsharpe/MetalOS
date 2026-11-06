#include "doomkeys.h"

#include "doomgeneric.h"

#include "core_crt/ctype.h"
#include "core_crt/string.h"
#include <stdio.h>

#include "user/MetalOS.h"
#include "user/MetalOS.UI.h"
#include "Graphics/Icons.h"
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

//Doom draws its 320x200 frame scaled by a whole number into the top of DG_ScreenBuffer, centered across, and leaves the rest black; the window is just that picture
static constexpr uint32_t PictureScale = (DOOMGENERIC_RESX / 320) < (DOOMGENERIC_RESY / 200) ? (DOOMGENERIC_RESX / 320) : (DOOMGENERIC_RESY / 200);
static constexpr uint32_t PictureWidth = 320 * PictureScale;
static constexpr uint32_t PictureHeight = 200 * PictureScale;
static constexpr uint32_t PictureLeft = (DOOMGENERIC_RESX - PictureWidth) / 2;
static pixel_t* s_Surface;

static void ProcessMessage(const struct Message &message)
{
	switch (message.Header.MessageType)
	{
		//PaintEvent needs no handling: DG_DrawFrame copies each frame into the window's surface, and the WM composites it.
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

		//A fullscreen window the size of Doom's picture, which the WM scales to fill the screen; Doom keeps drawing into the DG_ScreenBuffer doomgeneric_Create made
		Rectangle frame = {};
		frame.Width = PictureWidth;
		frame.Height = PictureHeight;

		AllocWindow(&window, &frame, "Doom", WindowStyle::Fullscreen, &Icons::Skull);
		DebugPrintf("Doom window: 0x%x x 0x%x\n", frame.Width, frame.Height);

		s_Surface = (pixel_t*)GetWindowSurface(window);
		Assert(s_Surface);

		memset(s_KeyQueue, 0, KEYQUEUE_SIZE * sizeof(unsigned short));
	}

	void DG_DrawFrame()
	{
		//Copy the picture out of Doom's buffer; the WM composites it on its next pass
		for (uint32_t y = 0; y < PictureHeight; y++)
			memcpy(s_Surface + (size_t)y * PictureWidth, DG_ScreenBuffer + (size_t)y * DOOMGENERIC_RESX + PictureLeft, PictureWidth * sizeof(pixel_t));

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
