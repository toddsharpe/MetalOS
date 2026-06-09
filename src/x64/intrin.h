#pragma once

// Collection of intrinsics from Windows intrin.h
// x64 only

extern "C"
{
	void __cpuid(int[4], int);
	void __halt(void);

	unsigned __int64 __readcr0(void);
	void __writecr0(unsigned __int64);

	unsigned __int64 __readcr2(void);
	void __writecr2(unsigned __int64);

	unsigned __int64 __readcr3(void);
	void __writecr3(unsigned __int64);

	unsigned __int64 __readcr4(void);
	void __writecr4(unsigned __int64);

	void __writecr8(unsigned __int64);
	unsigned __int64 __readcr8(void);

	void __writedr(unsigned int, unsigned __int64);
	unsigned __int64 __readdr(unsigned int);

	void _sgdt(void *);
	void __sidt(void *);

	unsigned char __inbyte(unsigned short);
	void __outbyte(unsigned short, unsigned char);

	unsigned short __inword(unsigned short);
	void __outword(unsigned short, unsigned short);

	unsigned long __indword(unsigned short);
	void __outdword(unsigned short, unsigned long);

	extern void __cdecl _writefsbase_u32(unsigned int);
	extern void __cdecl _writegsbase_u32(unsigned int);
	extern void __cdecl _writefsbase_u64(unsigned __int64);
	extern void __cdecl _writegsbase_u64(unsigned __int64);

	void _lgdt(void *);
	void __lidt(void *);
	void __writemsr(unsigned long, unsigned __int64);
	unsigned __int64 __readmsr(unsigned long);

	unsigned __int64 __rdtsc(void);

	__int64 _mul128(__int64 _Multiplier, __int64 _Multiplicand, __int64 * _HighProduct);

	unsigned __int64 __cdecl _readgsbase_u64(void);
	void __cdecl _writegsbase_u64(unsigned __int64);

	unsigned __int64 __readgsqword(unsigned long);

	short _InterlockedCompareExchange16(short volatile * _Destination, short _Exchange, short _Comparand);
	long _InterlockedCompareExchange(long volatile * _Destination, long _Exchange, long _Comparand);
	__int64 _InterlockedCompareExchange64(__int64 volatile * _Destination, __int64 _Exchange, __int64 _Comparand);
	__int64 _InterlockedIncrement64(__int64 volatile * _Addend);
	__int64 _InterlockedExchange64(__int64 volatile * _Target, __int64 _Value);
	long _InterlockedIncrement(long volatile * _Addend);

	void __faststorefence(void);

	void __invlpg(void *);
}

inline unsigned short _byteswap_ushort(unsigned short v)
{
	return (unsigned short)((v >> 8) | (v << 8));
}

inline unsigned long _byteswap_ulong(unsigned long v)
{
	return ((v & 0xFF000000UL) >> 24) | ((v & 0x00FF0000UL) >> 8)
	     | ((v & 0x0000FF00UL) << 8)  | ((v & 0x000000FFUL) << 24);
}

inline unsigned __int64 _byteswap_uint64(unsigned __int64 v)
{
	return ((unsigned __int64)_byteswap_ulong((unsigned long)v) << 32)
	     | (unsigned __int64)_byteswap_ulong((unsigned long)(v >> 32));
}
