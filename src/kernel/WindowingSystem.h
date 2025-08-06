#pragma once

#include "Lib/ObjectPool.h"
#include "Lib/StaticStack.h"
#include "Graphics/Types.h"
#include "user/MetalOS.Types.h"
#include "kernel/UWindow.h"
#include "Lib/StaticQueue.h"
#include "kernel/Objects/KSpinLock.h"

//Simple windowing system. Windows are drawn in order, top window is back of m_windows.
class UThread;
class WindowingSystem
{
private:
	static uint32_t ThreadLoop(void* arg);

public:
	WindowingSystem(Graphics::FrameBuffer& display);

	void Initialize();
	UWindow* CreateWindow(UThread& owner);
	bool Delete(UWindow* window);

	//Messages
	void PostMessage(Message& message);

	bool Enabled;

private:
	uint32_t ThreadLoop();

	void ProcessMessages();
	UWindow* GetWindow(const Graphics::Point2D& point) const;

	//Display/back buffer
	Graphics::FrameBuffer& m_display;
	Graphics::FrameBuffer m_backBuffer;

	//Window pool/list
	ListHead2<UWindow> m_windows;
	ObjectPool<UWindow, 16> m_windowPool;

	//Message queue
	KSpinLock m_messagesLock;
	StaticQueue<Message, 16> m_messages;

	//Mouse
	Graphics::Point2D m_mousePos;
	MouseButtonState m_prevMouseButtons;

	//Drag
	UWindow* m_dragWindow;
	UWindow* m_focusWindow;

	uint32_t m_postCount;
	uint32_t m_dropCount;
};

