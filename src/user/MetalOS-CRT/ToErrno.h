#pragma once

#include "user/MetalOS.h"
#include "errno.h"

//Closest errno for an RT call that failed
inline int ToErrno(const SyscallResult result)
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
