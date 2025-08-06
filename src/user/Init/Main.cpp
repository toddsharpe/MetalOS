#include "user/MetalOS.h"

int main(int argc, char** argv)
{
	DebugPrintf("Init process\n");

	//Launch demo apps
	//AssertSuccess(CreateProcess("calc.exe", nullptr, nullptr));
	AssertSuccess(CreateProcess("term.exe", nullptr, nullptr));
	//AssertSuccess(CreateProcess("term.exe", nullptr, nullptr));

	return 0;
}
