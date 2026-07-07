#include "user/MetalOS.h"

int main(int argc, char** argv)
{
	DebugPrintf("Init process\n");

	//Launch the usermode window manager first so it owns the framebuffer and
	//serves the "wm" endpoint before any GUI app connects.
	AssertSuccess(CreateProcess("wm.exe", nullptr, nullptr));

	//Launch the usermode network stack so it claims the kernel RX ring + NIC and
	//serves the "net" endpoint before any networking app connects.
	AssertSuccess(CreateProcess("netstack.exe", nullptr, nullptr));

	//Launch demo apps
	//AssertSuccess(CreateProcess("calc.exe", nullptr, nullptr));
	AssertSuccess(CreateProcess("term.exe", nullptr, nullptr));
	//AssertSuccess(CreateProcess("fire.exe", nullptr, nullptr));

	return 0;
}
