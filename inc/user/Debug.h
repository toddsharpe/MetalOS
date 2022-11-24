#pragma once

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  "Assert: " #x); }
#define AssertSuccess(x) if (!(x == SystemCallResult::Success)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  "Assert: " #x ", Result: 0x%x", x); }

#ifdef __cplusplus
extern "C" {
#endif

	void DebugBreak();
	void DebugPrintf(const char* format, ...);
	void __declspec(noreturn) Bugcheck(const char* file, const char* line, const char* format, ...);

#ifdef __cplusplus
}
#endif

