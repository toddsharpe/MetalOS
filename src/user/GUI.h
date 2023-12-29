#pragma once

#include "UI/Window.h"
#include <user/MetalOS.h>
#include "UserFrameBuffer.h"

class GUI;

typedef bool(*MessageCallback)(GUI& window, Message& message);

class GUI: public UI::Window
{
public:
	GUI(const std::string& title, const Graphics::Rectangle& rect, UI::WindowStyle& style, const MessageCallback& callback) : 
		Window(title, style),
		m_frameBuffer(rect),
		m_rect(rect),
		m_callback(callback),
		m_prevMouseButtons(),
		m_handle()
	{

	}

	void Initialize()
	{
		AssertSuccess(AllocWindow(m_handle, m_rect));
		m_frameBuffer.Initialize();
	}

	void Run()
	{
		Message message = {};
		while (GetMessage(message) == SystemCallResult::Success)
		{
			if (!m_callback(*this, message))
				DefaultCallback(message);
		}
	}

	void DefaultCallback(Message& message)
	{
		switch (message.Header.MessageType)
		{
		case MessageType::PaintEvent:
			Draw(m_frameBuffer);
			AssertSuccess(PaintWindow(m_handle, {m_frameBuffer.GetBuffer(), m_frameBuffer.Size()}));
			break;
		case MessageType::MouseEvent:
			//todo: clicked?
			if (message.MouseEvent.Buttons.LeftPressed && !m_prevMouseButtons.LeftPressed)
				DebugPrintf("%s: Mouse clicked\n", Title.c_str());

			m_prevMouseButtons = message.MouseEvent.Buttons;
			break;
		}
	}

	void GetRectangle(Graphics::Rectangle& frame)
	{
		//GetWindowRect?
		frame = m_rect;
	}

	void Move(const Graphics::Rectangle& frame)
	{
		//Implement these later
		Assert(frame.Height == m_rect.Height);
		Assert(frame.Width == m_rect.Width);

		MoveWindow(m_handle, frame);
		this->m_rect = frame;
	}

	HWindow GetHandle() const
	{
		return m_handle;
	}

	UserFrameBuffer& GetFrameBuffer()
	{
		return m_frameBuffer;
	}

private:
	UserFrameBuffer m_frameBuffer;
	Graphics::Rectangle m_rect;
	const MessageCallback& m_callback;

	MouseButtonState m_prevMouseButtons;
	HWindow m_handle;
};
