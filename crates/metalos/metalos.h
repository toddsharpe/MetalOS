enum SystemCallResult
{
	Success = 0,
	InvalidPointer,
	InvalidHandle,
	InvalidObject,
	BrokenPipe,
	Failed,
	NotImplemented
};

extern enum SystemCallResult DebugPrint(const char* s);
extern int printf(char const* const _Format);