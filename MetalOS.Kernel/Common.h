#pragma once

#define NO_COPY_OR_ASSIGN(X) X(const X&) = delete; X& operator = (const X&) = delete;
#define Assert(x) if (!(x)) { KernelBugcheck(#x); }
#define Fatal(x) KernelBugcheck(x);


