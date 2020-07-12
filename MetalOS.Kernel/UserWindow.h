#pragma once

#include "msvc.h"
#include <string>
#include <MetalOS.h>
#include <list>

class UserThread;
class UserWindow
{
public:
	UserWindow(const char* name, const Rectangle& rect, UserThread& owner);

	void PostMessage(Message* msg);
	
	const Rectangle& GetRectangle() const
	{
		return m_rect;
	}

private:
	std::string m_name;
	Rectangle m_rect;
	UserThread& m_owner;
};

