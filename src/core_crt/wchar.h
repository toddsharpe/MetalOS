#include "core_crt/stdint.h"
#include "core_crt/stdarg.h"

#ifdef __cplusplus
extern "C"
{
#endif

	int wcscmp(const wchar_t *str1, const wchar_t *str2);

	size_t wcslen(const wchar_t *str);

	wchar_t *wcscpy(wchar_t *dest, const wchar_t *source);

	wchar_t *wcsncpy(wchar_t *dest, const wchar_t *source, size_t num);

	inline int imax(int a, int b);
	/*
	 * Put a NUL-terminated ASCII number (base <= 36) in a buffer in reverse
	 * order; return an optional length and a pointer to the last character
	 * written in the buffer (i.e., the first character of the string).
	 * The buffer pointed to by `nbuf' must have length >= MAXNBUF.
	 */
	wchar_t *kswprintn(wchar_t *nbuf, uintmax_t num, int base, int *lenp, int upper);

	/*
	 * Scaled down version of printf(3).
	 *
	 * Two additional formats:
	 *
	 * The format %b is supported to decode error registers.
	 * Its usage is:
	 *
	 *	printf("reg=%b\n", regval, "<base><arg>*");
	 *
	 * where <base> is the output base expressed as a control character, e.g.
	 * \10 gives octal; \20 gives hex.  Each arg is a sequence of characters,
	 * the first of which gives the bit number to be inspected (origin 1), and
	 * the next characters (up to a control character, i.e. a character <= 32),
	 * give the name of the register.  Thus:
	 *
	 *	kvprintf("reg=%b\n", 3, "\10\2BITTWO\1BITONE");
	 *
	 * would produce output:
	 *
	 *	reg=3<BITTWO,BITONE>
	 *
	 * XXX:  %D  -- Hexdump, takes pointer and separator string:
	 *		("%6D", ptr, ":")   -> XX:XX:XX:XX:XX:XX
	 *		("%*D", len, ptr, " " -> XX XX XX XX ...
	 */
	int kvwprintf(wchar_t const *fmt, void (*func)(int, void *), void *arg, int radix, va_list ap);

	int swprintf(wchar_t *const _Buffer, size_t const _BufferCount, wchar_t const *const _Format, ...);

	// TODO: use _BufferCount
	int vswprintf(wchar_t *const _Buffer, size_t const _BufferCount, wchar_t const *const _Format, va_list _ArgList);

#ifdef __cplusplus
}
#endif
