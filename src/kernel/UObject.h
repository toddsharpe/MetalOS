#pragma once

#include "kernel/Types.h"
#include "kernel/Objects/KEvent.h"
#include "Assert.h"

enum class UObjectType
{
	Thread,
	Process,
	Window,

	File,
	Pipe,
	Debug,

	Semaphore,
	Event,

	Socket
};

typedef uintptr_t handle_t;
namespace Handles
{
	static constexpr handle_t StdIn = (handle_t)0;
	static constexpr handle_t StdOut = (handle_t)1;
	static constexpr handle_t StdErr = (handle_t)2;
	static constexpr handle_t Count = (handle_t)3;
}

class UThread;
class UProcess;
class UWindow;
class UPipe;
class KSocket;
class UObject
{
public:
	constexpr UObject(const UObjectType type);
	constexpr UObject(const UObjectType type, const handle_t handle);

	bool CanRead() const;
	bool CanWrite() const;
	void Display() const;

	// UObject
	const handle_t Handle;
	const UObjectType Type;

	//Depending on the type
	UThread* Thread;
	UProcess* Process;
	UWindow* Window;

	KFile File;
	UPipe* Pipe;

	KEvent* Event;

	KSocket* Socket;

private:
	static constexpr handle_t StartingHandle = 0x10;
	static handle_t LastHandle;
};
