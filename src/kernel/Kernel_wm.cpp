#include "Assert.h"
#include "kernel/Api.h"
#include "kernel/UWindow.h"
#include "kernel/UThread.h"
#include "kernel/Objects/KSpinLock.h"
#include "Lib/StaticQueue.h"
#include "Lib/LinkedList.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "user/MetalOS.Types.h"
#include "new.h"

using namespace Graphics;

//Simple windowing system. The top window is the head of m_windows (tail is the
//bottom of the z-order). Windows are heap allocated with KeAlloc. State and helpers
//live in the _WM namespace; m_display is defined in Kernel.cpp and in scope because
//this file is included after its namespace.
namespace _WM
{
	//Display/back buffer (m_backBuffer is sized from m_display at static init)
	FrameBuffer m_backBuffer(nullptr, m_display.Height, m_display.Width);

	//Window list, ordered top (head) to bottom (tail)
	LinkedList<UWindow*> m_windows;

	//Message queue
	KSpinLock m_messagesLock;
	StaticQueue<Message, 16> m_messages;

	//Mouse
	Point2D m_mousePos;
	MouseButtonState m_prevMouseButtons;

	//Drag/focus
	UWindow* m_dragWindow;
	UWindow* m_focusWindow;

	uint32_t m_postCount;
	uint32_t m_dropCount;

	bool m_enabled = false;

	UWindow* GetWindow(const Point2D& point)
	{
		//Topmost window is the first match scanning top (head) to bottom (tail)
		for (UWindow* window : m_windows)
		{
			if (window->GetBounds().Contains(point))
				return window;
		}
		return nullptr;
	}

	void ProcessMessages()
	{
		// Drain the queue under a single brief lock window so interrupt context
		// (KePostMessage) never spins waiting for us to finish processing a message.
		Message pending[16] = {};
		size_t pendingCount = 0;

		const cpu_flags_t flags = m_messagesLock.Acquire();
		while (!m_messages.IsEmpty())
			pending[pendingCount++] = m_messages.Dequeue();
		m_messagesLock.Release(flags);

		for (size_t i = 0; i < pendingCount; i++)
		{
			const Message& message = pending[i];
			switch (message.Header.MessageType)
			{
			case MessageType::MouseEvent:
				//Save cursor position
				const uint16_t absX = m_display.Width * message.MouseEvent.XPosition / INT16_MAX;
				const uint16_t absY = m_display.Height * message.MouseEvent.YPosition / INT16_MAX;
				const Point2D mousePos = { absX , absY };

				const bool clicked = message.MouseEvent.Buttons.LeftPressed && !m_prevMouseButtons.LeftPressed;

				//Detect click
				if (clicked)
				{
					m_focusWindow = GetWindow(mousePos);
					m_dragWindow = m_focusWindow;

					if (m_focusWindow != nullptr)
					{
						//Move to top (head of the list)
						m_windows.Remove(m_focusWindow);
						Assert(m_windows.InsertHead(m_focusWindow));
					}
				}
				//Detect drag
				else if (message.MouseEvent.Buttons.LeftPressed)
				{
					if (m_dragWindow)
					{
						//Calculate difference between old mouse and new mouse
						const int deltaX = (int)(mousePos.X - m_mousePos.X);
						const int deltaY = (int)(mousePos.Y - m_mousePos.Y);

						//Update window position
						m_dragWindow->Point.X += deltaX;
						m_dragWindow->Point.Y += deltaY;
					}
				}
				else
				{
					m_dragWindow = nullptr;
				}

				//Save previous mouse
				m_mousePos = mousePos;
				m_prevMouseButtons = message.MouseEvent.Buttons;
				break;
			}

			if (m_focusWindow)
			{
				m_focusWindow->Thread.Enqueue(message);
			}
		}
	}

	uint32_t ThreadLoop(void* arg)
	{
		UNUSED(arg);

		while (true)
		{
			if (m_enabled)
			{
				//Clear screen
				Draw2D::FillScreen(m_backBuffer, Colors::Black);

				//Process current messages
				ProcessMessages();

				//Draw windows back to front: bottom (tail) up to top (head)
				for (UWindow* window : m_windows.Reversed())
				{
					if (window == m_focusWindow)
					{
						Draw2D::WriteFrame(m_backBuffer, window->GetBounds(), window->Frame.Buffer);
					}
					else
					{
						Draw2D::WriteFrameGrayscale(m_backBuffer, window->GetBounds(), window->Frame.Buffer);
					}
				}

				//Write to graphics device
				memcpy(m_display.Buffer, m_backBuffer.Buffer, m_backBuffer.Size());

				//Request repaints
				//TODO(tsharpe): Only changed regions
				for (UWindow* window : m_windows)
				{
					Message message = {};
					message.Header.MessageType = MessageType::PaintEvent;
					message.PaintEvent.Region = window->GetBounds();
					window->Thread.Enqueue(message);
				}
			}

			KeSleepThread(Second / 30);
		}

		return 0;
	}
}

void KeWindowingInitialize()
{
	//Initialize back buffer
	_WM::m_backBuffer.Buffer = (Color*)KeVirtualAlloc(_WM::m_backBuffer.Size());

	//Create compositor thread
	KeCreateThread(&_WM::ThreadLoop, nullptr, "WindowingSystem::ThreadLoop");
}

void KeWindowingEnable()
{
	_WM::m_enabled = true;
}

UWindow* CreateWindow(UThread& owner)
{
	//Allocate window
	UWindow* window = KeAlloc<UWindow>(AllocType::Kernel, owner);
	Assert(window);

	//Add to top of list (head)
	Assert(_WM::m_windows.InsertHead(window));

	//Set it as focus
	_WM::m_focusWindow = window;

	return window;
}

void Delete(UWindow& window)
{
	//If window is focused or dragged clear it
	if (_WM::m_dragWindow == &window)
		_WM::m_dragWindow = nullptr;
	if (_WM::m_focusWindow == &window)
		_WM::m_focusWindow = nullptr;

	//Remove from list and free
	_WM::m_windows.Remove(&window);
	KeFree(&window, AllocType::Kernel);
}

void KePostMessage(Message& msg)
{
	const cpu_flags_t flags = _WM::m_messagesLock.Acquire();
	if (!_WM::m_messages.Enqueue(msg))
		_WM::m_dropCount++;
	else
		_WM::m_postCount++;
	_WM::m_messagesLock.Release(flags);
}
