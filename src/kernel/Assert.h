#pragma once

void Bugcheck(const char* file, const char* line, const char* format, ...);
void Printf(const char* format, ...);

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define AssertEval(x, f) if (!(x)) { Printf(__FILE__ "-" STR(__LINE__) "\n"); f; Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define AssertPrintInt(x, y) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), "Assert Failed: %s, %s=%x\n", #x, #y, y); }
#define AssertEqual(x, y) \
	if (!(x == y)) \
	{ \
		Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), #x " (0x%x) != " #y " (0x%x)", x, y); \
	}
#define AssertEqualEval(x, y, f) \
	if (!(x == y)) \
	{ \
		f; \
		Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), #x " (0x%x) != " #y " (0x%x)", x, y); \
	}
#define AssertNotEqual(x, y) \
	if (!(x != y)) \
	{ \
		Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), #x " (0x%x) == " #y " (0x%x)", x, y); \
	}
#define AssertOp(x, op, y) \
	if (!((x) op (y))) \
	{ \
		Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), #x " (0x%x) " STR(op) " " #y " (0x%x)", (x), (y)); \
	}
#define Fatal(x) Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); 
#define Trace() Printf(__FILE__ "-" STR(__LINE__) "\n");
