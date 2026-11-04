#include "kernel/Arch_x64/StackWalk.h"
#include <intrin.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

//Leaves the caller's RIP and RSP as they are right after this returns, as a fault there would
static __declspec(noinline) void StackWalk_test_capture(CONTEXT& context)
{
	context.Rip = (DWORD64)_ReturnAddress();
	context.Rsp = (DWORD64)_AddressOfReturnAddress() + sizeof(DWORD64);
}

//Unwinds this test's own stack the way PrintStack walks a user process's, checking every read against an address space
void StackWalk_test()
{
	const uintptr_t imageBase = (uintptr_t)&__ImageBase;
	const size_t imageSize = WinPE::GetSizeOfImage(&__ImageBase);

	CONTEXT context = {};
	StackWalk_test_capture(context);

	//This image and the stack above this frame, as a process's space holds its image and its thread's stack
	AddressSpace space(0, 0x00007FFF'FFFF0000);
	Assert(space.Reserve(imageBase, (imageSize + Arch::PageMask) >> Arch::PageShift));
	Assert(space.Reserve(context.Rsp & ~Arch::PageMask, 16));

	//Through main and the CRT's startup, until the walk leaves this image
	StackWalk walk(&context, &space);
	AssertEqual(walk.LookupIp(), context.Rip); //The first frame is looked up at its IP, as a fault's is
	size_t frames = 0;
	while (context.Rip >= imageBase && context.Rip < imageBase + imageSize)
	{
		Assert(walk.Next(imageBase));
		frames++;
		Assert(frames < 64);

		//Each caller is looked up inside its call instruction, not at the return address after it
		AssertEqual(walk.LookupIp(), context.Rip - 1);
	}
	Assert(frames >= 2);

	//A smashed RSP, outside the space (and unmapped, so reading it would crash this test), ends the walk
	CONTEXT smashed = {};
	StackWalk_test_capture(smashed);
	smashed.Rsp = 0x1000;
	Assert(!StackWalk(&smashed, &space).Next(imageBase));

	//The same for an IP with no unwind data, whose return address is popped straight off the stack
	CONTEXT leaf = {};
	leaf.Rip = imageBase + 0x10;
	leaf.Rsp = 0x1000;
	Assert(!StackWalk(&leaf, &space).Next(imageBase));
}
