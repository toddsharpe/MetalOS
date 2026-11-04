#pragma once

#include "user/MetalOS.h"
#include "x64/intrin.h"
#include "core_crt/stddef.h"
#include "errno.h"

namespace
{
	//Closest errno for an RT call that failed
	int ToErrno(const SyscallResult result)
	{
		switch (result)
		{
		case SyscallResult::InvalidPointer:
			return EFAULT;

		case SyscallResult::InvalidArg:
			return EINVAL;

		case SyscallResult::InvalidHandle:
		case SyscallResult::InvalidObject:
			return EBADF;

		case SyscallResult::BrokenPipe:
			return EPIPE;

		case SyscallResult::NotImplemented:
			return ENOSYS;

		default:
			return EIO;
		}
	}
}

extern "C"
{
	//errno lives in the calling thread's TEB; RT's GetTEB isn't exported, so read it the same way
	int* _errno(void)
	{
		ThreadEnvironmentBlock* const teb = (ThreadEnvironmentBlock*)__readgsqword(offsetof(ThreadEnvironmentBlock, SelfPointer));
		return &teb->Errno;
	}
}
