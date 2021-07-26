#pragma once

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define AssertEqual(x, y) \
	if (!(x == y)) \
	{ \
		kernel.BugcheckEx("File: " __FILE__, "Line: " STR(__LINE__), #x " (%d) != " #y " (%d)", x, y); \
	}
#define Fatal(x) kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); 
#define Trace() kernel.Print(__FILE__ "-" STR(__LINE__));
