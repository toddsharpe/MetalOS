#pragma once

typedef char* va_list;

void __cdecl __va_start(va_list* , ...);

#define __crt_va_start_a(ap, x) ((void)(__va_start(&ap, x)))
#define __crt_va_arg(ap, t)                                               \
    ((sizeof(t) > sizeof(__int64) || (sizeof(t) & (sizeof(t) - 1)) != 0) \
        ? **(t**)((ap += sizeof(__int64)) - sizeof(__int64))             \
        :  *(t* )((ap += sizeof(__int64)) - sizeof(__int64)))
#define __crt_va_end(ap)        ((void)(ap = (va_list)0))

#define __crt_va_start(ap, x) __crt_va_start_a(ap, x)

#define va_start __crt_va_start
#define va_arg   __crt_va_arg
#define va_end   __crt_va_end
#define va_copy(destination, source) ((destination) = (source))