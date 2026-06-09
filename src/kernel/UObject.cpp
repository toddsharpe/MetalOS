#pragma once

#include "kernel/UObject.h"
#include "kernel/Types.h"
#include "kernel/Objects/UPipe.h"

uintptr_t UObject::LastHandle = UObject::StartingHandle;

constexpr UObject::UObject(const UObjectType type) : UObject(type, LastHandle++)
{

}

constexpr UObject::UObject(const UObjectType type, const handle_t handle) :
	Handle(handle),
	Type(type),
	Thread(),
	Process(),
	Window(),
	File(),
	Pipe(),
	Event()
{

}

bool UObject::CanRead() const
{
	switch (Type)
	{
		case UObjectType::File:
			return (File.Access & KFileAccess::Read) == KFileAccess::Read;

		case UObjectType::Pipe:
			return Pipe->Op == KPipeOp::Read;

		case UObjectType::Debug:
			return false;

		default:
			return false;
	}
}

bool UObject::CanWrite() const
{
	switch (Type)
	{
		case UObjectType::File:
			return (File.Access & KFileAccess::Write) == KFileAccess::Write;

		case UObjectType::Pipe:
			return Pipe->Op == KPipeOp::Write;

		case UObjectType::Debug:
			return true;

		default:
			return false;
	}
}

void UObject::Display() const
{
	Printf("UObject::Display - 0x%016x\n", this);
	Printf("  Type: %d\n", Type);

	switch (Handle)
	{
		case Handles::StdIn:
			Printf("Handle: StdIn %d\n", Handle);
			break;

		case Handles::StdOut:
			Printf("Handle: StdOut %d\n", Handle);
			break;

		case Handles::StdErr:
			Printf("Handle: StdErr %d\n", Handle);
			break;

		default:
			Printf("Handle: %d\n", Handle);
			break;
	}

	Printf("     R: %d\n", CanRead());
	Printf("     W: %d\n", CanWrite());

	switch (Type)
	{
		case UObjectType::Thread:
			Assert(Thread);
			Printf("  UThread\n");
			Printf("    Name: %s\n", Thread->Name.c_str());
			break;
		
		case UObjectType::Process:
			Assert(Process);
			Printf("  UProcess\n");
			Printf("    Name: %s\n", Process->Name.c_str());
			break;

		case UObjectType::Window:
			Printf("  UWindow\n");
			Printf("    Thread.Name: %s\n", Window->Thread.Name.c_str());
			Printf("    Frame\n");
			Printf("    - Buffer: 0x%016x\n", Window->Frame.Buffer);
			Printf("    - Height,Width: %d,%d\n", Window->Frame.Height, Window->Frame.Width);
			Printf("    Point: (%d,%d)\n", Window->Point.X, Window->Point.Y);
			break;

		case UObjectType::File:
			Printf("  UFile\n");
			Printf("    Position: %d\n", File.Position);
			Printf("    Length: %d\n", File.Length);
			break;

		case UObjectType::Pipe:
			Printf("  UReadPipe\n");
			Printf("    Readers: %d\n", Pipe->KPipe.Readers.Count());
			Printf("    Writers: %d\n", Pipe->KPipe.Writers.Count());
			break;

		case UObjectType::Debug:
			Printf("  UFile\n");
			break;

		case UObjectType::Event:
			Printf("  UEvent\n");
			Printf("    Signalled: %d\n", Event->IsSignalled());
			break;

		default:
			Printf("Unknown type");
			break;
	}
}
