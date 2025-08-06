#pragma once

#include "Lib/List.h"
#include "Graphics/Types.h"
#include "kernel/UThread.h"

class UThread;
class UWindow
{
public:
	UWindow(UThread& thread) :
		Link(),
		Thread(thread),
		Frame(),
		Point()
	{

	};

	Graphics::Rectangle GetBounds() const
	{
		return { Point.X, Point.Y, Frame.Width, Frame.Height };
	}

	ListEntry Link;

	UThread& Thread;
	Graphics::FrameBuffer Frame;
	
	//Upper left
	Graphics::Point2D Point;
};
