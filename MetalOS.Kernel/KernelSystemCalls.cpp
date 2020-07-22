#include "Kernel.h"
#include "Main.h"
#include "UserWindow.h"

//TODO: kernel method to validate user pointer

SystemCallResult Kernel::GetSystemInfo(SystemInfo* info)
{
	if (!info)
		return SystemCallResult::Failed;
	
	info->PageSize = PAGE_SIZE;
	info->Architecture = SystemArchitecture::x64;
	return SystemCallResult::Success;
}

//Milliseconds
size_t Kernel::GetTickCount()
{
	const nano100_t tsc = m_hyperV->ReadTsc();
	return (tsc * 100) / 1000000;
}

void Kernel::Sleep(uint32_t milliseconds)
{
	if (!milliseconds)
		return;

	KernelThreadSleep((nano_t)milliseconds * 1000 * 1000);
}

SystemCallResult Kernel::ExitProcess(uint32_t exitCode)
{
	UserProcess& process = m_scheduler->GetCurrentProcess();
	Print("Process: %s exited with code 0x%x\n", process.GetName().c_str(), exitCode);
	m_scheduler->KillThread();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::CreateWindow(const char* name)
{
	if (!name)
		return SystemCallResult::Failed;

	//One window at at time
	Assert(Window == nullptr);

	Rectangle rect = { 0 };
	rect.P1.X = 0;
	rect.P1.Y = 0;
	rect.P2.X = m_display->GetWidth();
	rect.P2.Y = m_display->GetHeight();

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);

	UserWindow* window = new UserWindow(name, rect, *user);
	user->Window = window;
	this->Window = window;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::GetWindowRect(Handle handle, Rectangle* rect)
{
	if (!rect)
		return SystemCallResult::Failed;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	if (!user->Window)
		return SystemCallResult::Failed;

	//Copy rectangle
	*rect = user->Window->GetRectangle();

	return SystemCallResult::Success;
}

//Blocks until message and returns it
SystemCallResult Kernel::GetMessage(Message* message)
{
	if (!message)
		return SystemCallResult::Failed;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	if (!user->Window)
		return SystemCallResult::Failed;

	Message* msg = m_scheduler->MessageWait(); //Can block
	*message = *msg;
	return SystemCallResult::Success;
}

//Doesn't block
SystemCallResult Kernel::PeekMessage(Message* message)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);

	Message* msg = user->DequeueMessage();
	if (!msg)
		return SystemCallResult::Failed;
	
	*message = *msg;
	return SystemCallResult::Success;
}

void memcpy_sse(void* dest, const void* src, size_t count)
{
	__m128i* srcPtr = (__m128i*)src;
	__m128i* destPtr = (__m128i*)dest;

	unsigned int index = 0;
	while (count) {

		__m128i x = _mm_loadu_si128(&srcPtr[index]);
		_mm_stream_si128(&destPtr[index], x);

		count -= 16;
		index++;
	}
}

void* memcpy_512bit_as(void* dest, const void* src, size_t len)
{
	const __m512i* s = (__m512i*)src;
	__m512i* d = (__m512i*)dest;

	while (len--)
	{
		_mm512_stream_si512(d++, _mm512_stream_load_si512(s++));
	}
	_mm_sfence();

	return dest;
}

// 256 bytes
void* memcpy_128bit_256B_as(void* dest, const void* src, size_t len)
{
	__m128i* s = (__m128i*)src;
	__m128i* d = (__m128i*)dest;

	while (len--)
	{
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 1
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 2
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 3
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 4
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 5
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 6
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 7
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 8
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 9
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 10
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 11
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 12
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 13
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 14
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 15
		_mm_stream_si128(d++, _mm_stream_load_si128(s++)); // 16
	}
	_mm_sfence();

	return dest;
}

SystemCallResult Kernel::SetScreenBuffer(void* buffer)
{
	if (!buffer)
		return SystemCallResult::Failed;

	const size_t size = (size_t)m_display->GetHeight() * m_display->GetWidth();
	//Printf("D: 0x%016x, S: 0x%016x\n", (void*)m_display->Buffer(), buffer);
	memcpy((void*)m_display->Buffer(), buffer, sizeof(Color) * size);

	return SystemCallResult::Success;
}

Handle Kernel::CreateFile(const char* name, GenericAccess access)
{
	if (!name)
		return nullptr;

	Print("CreateFile: %s Access: %d\n", name, access);

	return this->CreateFile(std::string(name), access);
}

SystemCallResult Kernel::ReadFile(Handle* handle, void* buffer, size_t bufferSize, size_t* bytesRead)
{
	if (!handle || !buffer || !bufferSize)
		return SystemCallResult::Failed;

	FileHandle* file = (FileHandle*)handle;

	bool result = this->ReadFile(file, buffer, bufferSize, bytesRead);
	return result ? SystemCallResult::Success : SystemCallResult::Failed;
}

SystemCallResult Kernel::WriteFile(Handle hFile, const void* lpBuffer, size_t bufferSize, size_t* bytesWritten)
{
	//Not implemented
	Assert(false);
	SystemCallResult::Failed;
}

SystemCallResult Kernel::SetFilePointer(Handle* handle, __int64 position, FilePointerMove moveType, size_t* newPosition)
{
	if (!handle)
		return SystemCallResult::Failed;

	FileHandle* file = (FileHandle*)handle;
	
	size_t pos;
	switch (moveType)
	{
	case FilePointerMove::Begin:
		pos = position;
		break;

	case FilePointerMove::Current:
		pos = file->Position + position;
		break;

	case FilePointerMove::End:
		pos = file->Length + position;
		break;

	default:
		return SystemCallResult::Failed;
	}

	bool result = this->SetFilePosition(file, pos);
	if (result)
	{
		if (newPosition != nullptr)
			*newPosition = pos;
		return SystemCallResult::Success;
	}
	
	return SystemCallResult::Failed;
}

SystemCallResult Kernel::CloseFile(Handle* handle)
{
	if (!handle)
		return SystemCallResult::Failed;

	CloseFile((FileHandle*)handle);
	return SystemCallResult::Success;
}

size_t Kernel::MoveFile(const char* existingFileName, const char* newFileName)
{
	Assert(false);
	SystemCallResult::Failed;
}

SystemCallResult Kernel::DeleteFile(const char* fileName)
{
	Assert(false);
	SystemCallResult::Failed;
}

SystemCallResult Kernel::CreateDirectory(const char* path)
{
	Assert(false);
	SystemCallResult::Failed;
}

void* Kernel::VirtualAlloc(void* address, size_t size, MemoryAllocationType allocationType, MemoryProtection protect)
{
	if (!size)
		return nullptr;

	UserProcess& process = m_scheduler->GetCurrentProcess();
	
	return m_virtualMemory->Allocate((uintptr_t)address, SIZE_TO_PAGES(size), protect, process.GetAddressSpace());
}

SystemCallResult Kernel::DebugPrint(char* s)
{
	this->m_printer->Write(s);
	return SystemCallResult::Success;
}
