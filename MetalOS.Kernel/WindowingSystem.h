#pragma once
#include <MetalOS.h>
#include "MetalOS.Kernel.h"
#include "UserThread.h"
#include "Display.h"
#include "msvc.h"
#include <string>

class WindowingSystem
{
public:
	WindowingSystem(Display* display);
	void Initialize();

	HWindow AllocWindow(UserThread* thread, const std::string& name, const Rectangle& bounds);
	bool PaintWindow(HWindow handle, const ReadOnlyBuffer& buffer);
	bool MoveWindow(HWindow handle, const Rectangle& bounds);
	bool GetWindowRect(HWindow handle, Rectangle& bounds);
	bool ThreadHasWindow(size_t threadId);

	void PostMessage(Message* message);

private:
	static uint32_t ThreadLoop(void* arg) { return ((WindowingSystem*)arg)->ThreadLoop(); };
	uint32_t ThreadLoop();

	struct Window
	{
		Window(const std::string& title) : Title(title) {};
		
		Rectangle Bounds;
		Buffer FrameBuffer;
		const std::string& Title;
		UserThread* Thread;
		int ZIndex;
	};

	Window* GetWindow(size_t threadId)
	{

	}

	Window* GetWindow(Point2D point);

	std::list<Window*> m_windows;
	Display* m_display;
	Point2D m_mousePos;
	MouseButtonState m_prevMouseButtons;

	//Drag
	Window* m_dragWindow;
};

