#pragma once

#include "Graphics/Types.h"
#include "Graphics/Theme.h"

namespace UI
{
	using namespace Graphics;

	class Control
	{
	public:
		Control(const Rectangle& bounds) : Foreground(Theme::Bone), Background(Theme::Iron), Bounds(bounds), Hovered(), Pressed()
		{

		}

		virtual void Draw(FrameBuffer& frame) = 0;

		//The window got a left click inside Bounds; most controls ignore it
		virtual void OnClick()
		{
		}

		Color Foreground;
		Color Background;
		Rectangle Bounds; //in window coordinates
		bool Hovered; //the mouse is over it
		bool Pressed; //held down, by the mouse or a key standing in for it
	};
}
