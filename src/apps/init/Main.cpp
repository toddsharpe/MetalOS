#include <user/MetalOS.h>
#include <user/Debug.h>

int main(int argc, char** argv)
{
	DebugPrintf("Init process\n");

	//Launch demo apps
	AssertSuccess(CreateProcess("calc.exe", nullptr, nullptr));
	AssertSuccess(CreateProcess("fire.exe", nullptr, nullptr));
	AssertSuccess(CreateProcess("term.exe", nullptr, nullptr));
}
