#include "PageFrameNumberDatabase.h"

PageFrameNumberDatabase::PageFrameNumberDatabase(size_t count) : m_db(count), m_zeroed(nullptr), m_free(nullptr), m_standby(nullptr), m_modifed(nullptr)
{

}

uintptr_t PageFrameNumberDatabase::AllocatePage()
{
	if (m_free != nullptr)
	{
		m_free->PageLocation = PageLocation::Active;
		//m_free->
	}

	return 0;
}

uintptr_t PageFrameNumberDatabase::GetPFN(PPFN_ENTRY entry)
{
	return ((uintptr_t)entry - (uintptr_t)(&m_db.front())) / sizeof(PFN_ENTRY);
}



