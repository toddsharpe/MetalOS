#pragma once

#include "Lib/StaticVector.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "Graphics/Theme.h"
#include "UI/Control.h"
#include "user/MetalOS.h"
#include "Assert.h"

class GUI;

typedef bool (*MessageCallback)(GUI &window, Message &message);

//An app window: the WM draws its frame and title, so this draws only the client area and its controls
class GUI
{
public:
	GUI(const CString &title, const Graphics::Rectangle &rect, const MessageCallback callback, const WindowStyle style = WindowStyle::Framed, const Graphics::PixelGlyph* icon = nullptr) :
		Frame(nullptr, rect.Height, rect.Width),
		Background(Graphics::Theme::Iron),
		Children(),
		Title(title),
		m_callback(callback),
		m_point({rect.X, rect.Y}),
		m_style(style),
		m_icon(icon),
		m_prevMouseButtons(),
		m_pressed(),
		m_handle()
	{
	}

	void Initialize()
	{
		// Allocate
		Graphics::Rectangle rect = {m_point.X, m_point.Y, Frame.Width, Frame.Height};
		AssertSuccess(AllocWindow(&m_handle, &rect, Title.c_str(), m_style, m_icon));

		// Draw directly into the WM-provided shared surface (zero-copy)
		Frame.Buffer = (Graphics::Color *)GetWindowSurface(m_handle);
		Assert(Frame.Buffer);
	}

	//An app with more than one window runs its own loop instead, picking the window by message.Header.Window
	void Run()
	{
		Message message = {};
		while (GetMessage(&message) == SyscallResult::Success)
		{
			if (message.Header.Window != WindowId(m_handle))
				continue; //for another of the app's windows, which this loop doesn't serve
			if (message.Header.MessageType == MessageType::CloseEvent)
				break; //WM asked us to close; exit the message loop so main() can return

			//A new size comes first, so the app lays itself out on the new surface
			if (message.Header.MessageType == MessageType::ResizeEvent)
				Resize(message.ResizeEvent.Width, message.ResizeEvent.Height);

			if (!m_callback(*this, message))
				DefaultCallback(message);
		}
	}

	void DefaultCallback(Message &message)
	{
		switch (message.Header.MessageType)
		{
		case MessageType::PaintEvent:
		case MessageType::ResizeEvent:
			//Draw straight into the WM-provided shared surface; the WM composites it.
			Draw();
			break;

		case MessageType::MouseEvent:
			OnMouse(message.MouseEvent);
			break;
		}
	}

	HWindow Handle() const
	{
		return m_handle;
	}

	Graphics::FrameBuffer Frame;
	Graphics::Color Background;
	StaticVector<UI::Control*, 32> Children;
	StaticString<32> Title;

private:
	void Draw()
	{
		Graphics::Draw2D::FillScreen(Frame, Background);
		for (const auto& child : Children)
		{
			child->Draw(Frame);
		}
	}

	//Answers the WM's ResizeEvent with a surface of that size and draws from then on into it
	void Resize(const uint32_t width, const uint32_t height)
	{
		if (width == 0 || height == 0 || ResizeWindow(m_handle, width, height) != SyscallResult::Success)
			return;

		Frame = Graphics::FrameBuffer((Graphics::Color *)GetWindowSurface(m_handle), height, width);
	}

	//The child drawn last under a point in window coordinates, since it is the one on top
	UI::Control* HitTest(const Graphics::Point2D& point) const
	{
		for (size_t i = Children.Count(); i > 0; i--)
		{
			UI::Control* const child = Children[i - 1];
			if (child->Bounds.Contains(point))
				return child;
		}

		return nullptr;
	}

	//A click is the left button going down and coming back up over the same child; the child under the mouse shows as hovered
	void OnMouse(const MouseEvent& mouse)
	{
		const Graphics::Point2D point = { mouse.XPosition, mouse.YPosition };
		UI::Control* const under = HitTest(point);
		for (const auto& child : Children)
			child->Hovered = (child == under);

		if (mouse.Buttons.LeftPressed && !m_prevMouseButtons.LeftPressed)
		{
			m_pressed = under;
			if (m_pressed)
				m_pressed->Pressed = true;
		}
		else if (!mouse.Buttons.LeftPressed && m_prevMouseButtons.LeftPressed)
		{
			if (m_pressed)
			{
				m_pressed->Pressed = false;
				if (under == m_pressed)
					m_pressed->OnClick();
			}
			m_pressed = nullptr;
		}

		m_prevMouseButtons = mouse.Buttons;
	}

	MessageCallback m_callback;

	Graphics::Point2D m_point;
	WindowStyle m_style;
	const Graphics::PixelGlyph* m_icon;
	MouseButtonState m_prevMouseButtons;
	UI::Control* m_pressed;
	HWindow m_handle;
};
