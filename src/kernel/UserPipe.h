#pragma once
#include "KObject.h"

//Circular buffer using pages double mapped
//Methods return false if they cant be completed at once
//TODO: mutex/lock?
class UserPipe : public KObject
{
private:
	static constexpr size_t PageCount = 1;

public:
	UserPipe();

	bool Read(void* buffer, size_t length);
	size_t ReadPartial(void* buffer, size_t length);
	bool Write(const void* buffer, size_t length);

	size_t FreeSpace() const;
	size_t Count() const;

	virtual void Dispose() override;

	bool IsBroken() const
	{
		return !ReaderCount || !WriterCount;
	}

	size_t ReaderCount;
	size_t WriterCount;

private:
	size_t m_readIndex;
	size_t m_writeIndex;
	size_t m_count;
	size_t m_size;
	void* m_buffer;

	size_t m_readerCount;
	size_t m_writeCount;
};

