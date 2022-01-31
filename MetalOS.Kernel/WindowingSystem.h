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

	HWindow AllocWindow(UserThread* thread, const Rectangle& bounds);
	bool PaintWindow(const HWindow handle, const ReadOnlyBuffer& buffer);
	bool MoveWindow(const HWindow handle, const Rectangle& bounds);
	bool GetWindowRect(const HWindow handle, 
		Rectangle& bounds);
	bool ThreadHasWindow(const UserThread* thread) const;
	void FreeWindow(UserThread* thread);

	void PostMessage(Message* message);

private:
	static size_t ThreadLoop(void* arg);
	size_t ThreadLoop() const;

	struct Window
	{
		Window() :
			Bounds(),
			FrameBuffer(),
			Thread()
		{};
		
		Rectangle Bounds;
		Buffer FrameBuffer;
		UserThread* Thread;
	};

	Window* GetWindow(const Point2D& point) const;
	bool HandleValid(const HWindow handle) const;

	std::list<Window*> m_windows;
	Display* m_display;
	Point2D m_mousePos;
	MouseButtonState m_prevMouseButtons;

	//Drag
	Window* m_dragWindow;
	Window* m_focusWindow;
};

