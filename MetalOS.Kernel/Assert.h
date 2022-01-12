#pragma once

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define AssertPrintInt(x, y) if (!(x)) { kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), "Assert Failed: %s, %s=%d\n", #x, #y, y); }
#define AssertEqual(x, y) \
	if (!(x == y)) \
	{ \
		kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), #x " (0x%x) != " #y " (0x%x)", x, y); \
	}
#define AssertNotEqual(x, y) \
	if (!(x != y)) \
	{ \
		kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), #x " (0x%x) != " #y " (0x%x)", x, y); \
	}
#define AssertOp(x, op, y) \
	if (!(x op y)) \
	{ \
		kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__), #x " (0x%x) " STR(op) " " #y " (0x%x)", x, y); \
	}
#define Fatal(x) kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); 
#define Trace() kernel.Print(__FILE__ "-" STR(__LINE__));
