#pragma once

void Bugcheck(const char* file, const char* line, const char* format, ...);
void DebugPrintf(const char* format, ...);
void DebugBreak();

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  "Assert: " #x); }
#define AssertSuccess(x) \
	if (!(x == SystemCallResult::Success)) \
	{ \
		DebugPrintf("SystemCallResult: %d\n", x); \
		Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  "Assert: " #x ", Result: 0x%x", x); \
	}
