#include "WindowingSystem.h"

#include "Kernel.h"
#include "Assert.h"
#include <limits>

using namespace Graphics;

size_t WindowingSystem::ThreadLoop(void* arg)
{
	return static_cast<WindowingSystem*>(arg)->ThreadLoop();
};

WindowingSystem::WindowingSystem(EfiDisplay& display) :
	m_display(display),
	m_frameBuffer(),
	m_windows(),
	m_mousePos(),
	m_prevMouseButtons(),
	m_dragWindow(),
	m_focusWindow()
{

}

void WindowingSystem::Initialize()
{
	//Initialize Framebuffer
	m_frameBuffer.Initialize(m_display.GetHeight(), m_display.GetWidth());

	//Initialize window list
	m_windows = std::make_unique<std::list<Window>>();
	
	//Create thread
	KThread* thread = kernel.KeCreateThread(&WindowingSystem::ThreadLoop, this);
	thread->Name = "WindowingSystem::ThreadLoop";
}

HWindow WindowingSystem::AllocWindow(UserThread& thread, const Rectangle& bounds)
{
	Printf("AllocWIndow\n");
	
	//Allocate framebuffer on page boundaries
	//Map into process directly one day?
	const size_t bytes = bounds.Height * bounds.Width * sizeof(Color);
	const size_t count = SIZE_TO_PAGES(bytes);
	void* buffer = kernel.AllocateWindows(count);

	//Create window
	Window& window = m_windows->emplace_back(thread);
	window.Bounds = bounds;
	window.FrameBuffer = { buffer, bytes };

	return (HWindow)&window;
}

bool WindowingSystem::PaintWindow(const HWindow handle, const ReadOnlyBuffer& buffer)
{
	if (!HandleValid(handle))
		return false;

	Window* window = static_cast<Window*>(handle);
	if (window->FrameBuffer.Length != buffer.Length)
		return false;

	memcpy(window->FrameBuffer.Data, buffer.Data, buffer.Length);
	return true;
}

bool WindowingSystem::MoveWindow(const HWindow handle, const Rectangle& bounds)
{
	if (!HandleValid(handle))
		return false;

	//NOTE(tsharpe): Old buffers can't be deallocated yet.
	//Therefore resize isn't supported and windows are enforced to be the same size.
	Window* window = static_cast<Window*>(handle);
	Assert(bounds.Height == window->Bounds.Height);
	Assert(bounds.Width == window->Bounds.Width);
	window->Bounds = bounds;

	return true;
}

bool WindowingSystem::GetWindowRect(const HWindow handle, Rectangle& bounds)
{
	if (!HandleValid(handle))
		return false;

	Window* window = static_cast<Window*>(handle);
	bounds = window->Bounds;
	return true;
}

void WindowingSystem::PostMessage(Message* message)
{
	switch (message->Header.MessageType)
	{
	case MessageType::MouseEvent:
		//Save cursor position
		constexpr uint16_t maxX = std::numeric_limits<int16_t>::max();
		constexpr uint16_t maxY = std::numeric_limits<int16_t>::max();
		uint16_t absX = m_display.GetWidth() * message->MouseEvent.XPosition / maxX;
		uint16_t absY = m_display.GetHeight() * message->MouseEvent.YPosition / maxX;

		//Draw cursor
		Point2D mousePos = {};
		mousePos.X = absX;
		mousePos.Y = absY;

		//Detect click
		if (message->MouseEvent.Buttons.LeftPressed && !m_prevMouseButtons.LeftPressed)
		{
			Window* focus = GetWindow(mousePos);
			if (focus != nullptr)
			{
				//Remove from list, push on top
				m_windows->remove(*focus);
				m_windows->push_back(*focus);
			}
		}

		//Detect drag
		if (message->MouseEvent.Buttons.LeftPressed)
		{
			//Focus window
			Window* selected = GetWindow(mousePos);
			m_focusWindow = selected;

			if (m_dragWindow == nullptr)
			{
				m_dragWindow = GetWindow(mousePos);
			}

			//Verify we have a window to move
			if (m_dragWindow != nullptr)
			{
				//Calculate difference between old mouse and new mouse
				int deltaX = (int)(mousePos.X - m_mousePos.X);
				int deltaY = (int)(mousePos.Y - m_mousePos.Y);

				//Update window position
				m_dragWindow->Bounds.X += deltaX;
				m_dragWindow->Bounds.Y += deltaY;
			}
		}
		else
		{
			m_dragWindow = nullptr;
		}

		//Update old mouse position
		m_mousePos = mousePos;
		break;
	}

	if (m_focusWindow != nullptr)
	{
		m_focusWindow->Thread.EnqueueMessage(message);
	}
}

bool WindowingSystem::ThreadHasWindow(const UserThread& thread) const
{
	for (const Window& window : *m_windows)
	{
		if (&window.Thread == &thread)
			return true;
	}

	return false;
}

void WindowingSystem::FreeWindow(const UserThread& thread)
{
	for (const Window& window : *m_windows)
	{
		if (&window.Thread != &thread)
			continue;
		
		//Change focus
		if (m_focusWindow == &window)
			m_focusWindow = nullptr;
		if (m_focusWindow == &window)
			m_dragWindow = nullptr;

		m_windows->remove(window);
		return;
	}
	
	Assert(false);
}

size_t WindowingSystem::ThreadLoop()
{
	while (true)
	{
		m_frameBuffer.FillScreen(Colors::Black);

		//Draw windows
		for (const Window& window : *m_windows)
		{
			if (&window != m_focusWindow)
			{
				m_frameBuffer.WriteFrameGrayscale(window.Bounds, window.FrameBuffer.Data);
			}
			else
			{
				//Focus window gets color
				m_frameBuffer.WriteFrame(window.Bounds, window.FrameBuffer.Data);
			}
		}

		//Draw cursor
		//m_frameBuffer.DrawRectangle(Colors::Red, { m_mousePos.X, m_mousePos.Y, 3, 3 });
		m_frameBuffer.DrawCursor({ m_mousePos.X, m_mousePos.Y }, Colors::Red);

		//Send to graphics device
		m_display.Write(m_frameBuffer);

		//Request repaints
		//TODO: only changed regions?
		for (const Window& window : *m_windows)
		{
			Assert(!window.Thread.Deleted);

			UserProcess& proc = window.Thread.GetProcess();
			Assert(!proc.IsSignalled());

			Message message;
			message.Header.MessageType = MessageType::PaintEvent;
			message.PaintEvent.Region = window.Bounds;
			window.Thread.EnqueueMessage(&message);
		}

		kernel.KeSleepThread(SECOND / 30);
	}
}

WindowingSystem::Window* WindowingSystem::GetWindow(const Point2D& point) const
{
	//Loop through in reverse, finding first window inside bounds
	auto it = m_windows->rbegin();
	while (it != m_windows->rend())
	{
		Window& window = *it;
		if (window.Bounds.Contains(point))
			return &window;

		it++;
	}
	return nullptr;
}

bool WindowingSystem::HandleValid(const HWindow handle) const
{
	for (const Window& window : *m_windows)
	{
		if (&window == handle)
			return true;
	}
	return false;
}


