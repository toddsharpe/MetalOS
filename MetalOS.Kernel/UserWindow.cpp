#include "UserWindow.h"
#include "UserThread.h"

UserWindow::UserWindow(const char* name, const Rectangle& rect, UserThread& owner) :
	m_name(name),
	m_rect(rect),
	m_owner(owner)
{

}

void UserWindow::PostMessage(Message* msg)
{
	m_owner.EnqueueMessage(msg);
}

