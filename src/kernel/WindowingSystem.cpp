#include "WindowingSystem.h"

#include "Kernel/Kernel.h"
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
	m_windows = std::make_unique<std::list<std::shared_ptr<Window>>>();
	
	//Create thread
	kernel.KeCreateThread(&WindowingSystem::ThreadLoop, this, "WindowingSystem::ThreadLoop");
}

HWindow WindowingSystem::AllocWindow(UserThread& thread, const Rectangle& bounds)
{
	Printf("AllocWindow\n");
	
	//Allocate framebuffer on page boundaries
	//Map into process directly one day?
	const size_t bytes = bounds.Height * bounds.Width * sizeof(Color);
	const size_t count = SizeToPages(bytes);
	void* buffer = kernel.AllocateWindows(count);

	//Create window, focused
	std::shared_ptr<Window> window = std::make_shared<Window>(thread);
	window->Bounds = bounds;
	window->FrameBuffer = { buffer, bytes };

	m_windows->push_back(window);
	m_focusWindow = window;

	return static_cast<HWindow>(window.get());
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

void WindowingSystem::PostMessage(Message& message)
{
	switch (message.Header.MessageType)
	{
	case MessageType::MouseEvent:

		//Save cursor position
		constexpr uint16_t maxX = std::numeric_limits<int16_t>::max();
		constexpr uint16_t maxY = std::numeric_limits<int16_t>::max();
		uint16_t absX = m_display.GetWidth() * message.MouseEvent.XPosition / maxX;
		uint16_t absY = m_display.GetHeight() * message.MouseEvent.YPosition / maxX;
		Point2D mousePos = { absX , absY };

		//Detect focus/drag
		if (message.MouseEvent.Buttons.LeftPressed)
		{
			//Focus window
			m_focusWindow = GetWindow(mousePos);
			if (m_focusWindow)
			{
				m_windows->remove(m_focusWindow);
				m_windows->push_back(m_focusWindow);
			}

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
	for (const std::shared_ptr<Window>& window : *m_windows)
	{
		if (&window->Thread == &thread)
			return true;
	}

	return false;
}

void WindowingSystem::FreeWindow(const UserThread& thread)
{
	for (const std::shared_ptr<Window>& window : *m_windows)
	{
		if (&window->Thread != &thread)
			continue;
		
		//Release focus
		if (m_focusWindow == window)
			m_focusWindow.reset();
		if (m_dragWindow = window)
			m_dragWindow.reset();

		m_windows->remove(window);
		return;
	}
	
	Assert(false);
}

void WindowingSystem::FreeWindows(const UserProcess& proc)
{
	auto it = m_windows->begin();
	while (it != m_windows->end())
	{
		const std::shared_ptr<Window>& item = *it;
		if (&item->Thread.Process == &proc)
		{
			//Release focus
			if (m_focusWindow == item)
				m_focusWindow.reset();
			if (m_dragWindow = item)
				m_dragWindow.reset();
			
			it = m_windows->erase(it);
		}
		else
		{
			it++;
		}
	}
}

size_t WindowingSystem::ThreadLoop()
{
	while (true)
	{
		m_frameBuffer.FillScreen(Colors::Black);

		//Draw windows
		for (const std::shared_ptr<Window>& window : *m_windows)
		{
			if (window != m_focusWindow)
			{
				m_frameBuffer.WriteFrameGrayscale(window->Bounds, window->FrameBuffer.Data);
			}
			else
			{
				//Focus window gets color
				m_frameBuffer.WriteFrame(window->Bounds, window->FrameBuffer.Data);
			}
		}

		//Draw cursor
		m_frameBuffer.DrawCursor({ m_mousePos.X, m_mousePos.Y }, Colors::Red);

		//Send to graphics device
		m_display.Write(m_frameBuffer);

		//Request repaints
		//TODO(tsharpe): Only changed regions
		for (const std::shared_ptr<Window>& window : *m_windows)
		{
			Assert(!window->Thread.Deleted);

			UserProcess& proc = window->Thread.Process;
			Assert(!proc.IsSignalled());

			Message message = {};
			message.Header.MessageType = MessageType::PaintEvent;
			message.PaintEvent.Region = window->Bounds;
			window->Thread.EnqueueMessage(message);
		}

		kernel.KeSleepThread(Second / 30);
	}
}

std::shared_ptr<WindowingSystem::Window> WindowingSystem::GetWindow(const Point2D& point) const
{
	for (auto it = m_windows->rbegin(); it != m_windows->rend(); it++)
	{
		std::shared_ptr<Window>& window = *it;
		if (window->Bounds.Contains(point))
		{
			return window;
		}
	}

	return nullptr;
}

bool WindowingSystem::HandleValid(const HWindow handle) const
{
	for (const std::shared_ptr<Window>& window : *m_windows)
	{
		if (window.get() == handle)
			return true;
	}
	return false;
}


